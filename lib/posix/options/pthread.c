/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_clock.h"
#include "posix_internal.h"
#include "pthread_sched.h"

#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/elastipool.h>
#include <zephyr/sys/sem.h>
#include <zephyr/sys/slist.h>
#include <zephyr/sys/thread.h>
#include <zephyr/sys/util.h>

struct __pthread_cleanup {
	void (*routine)(void *arg);
	void *arg;
	sys_snode_t node;
};

#define ZEPHYR_TO_POSIX_PRIORITY(_zprio)                                                           \
	(((_zprio) < 0) ? (-1 * ((_zprio) + 1)) : (CONFIG_NUM_PREEMPT_PRIORITIES - (_zprio)-1))

#define POSIX_TO_ZEPHYR_PRIORITY(_prio, _pol)                                                      \
	(((_pol) == SCHED_FIFO) ? (-1 * ((_prio) + 1))                                             \
				: (CONFIG_NUM_PREEMPT_PRIORITIES - (_prio)-1))

#define DEFAULT_PTHREAD_PRIORITY                                                                   \
	POSIX_TO_ZEPHYR_PRIORITY(K_LOWEST_APPLICATION_THREAD_PRIO, DEFAULT_PTHREAD_POLICY)
#define DEFAULT_PTHREAD_POLICY (IS_ENABLED(CONFIG_PREEMPT_ENABLED) ? SCHED_RR : SCHED_FIFO)

#define PTHREAD_STACK_MAX BIT(CONFIG_POSIX_PTHREAD_ATTR_STACKSIZE_BITS)
#define PTHREAD_GUARD_MAX BIT_MASK(CONFIG_POSIX_PTHREAD_ATTR_GUARDSIZE_BITS)

#ifdef CONFIG_DYNAMIC_THREAD_STACK_SIZE
#define DYNAMIC_STACK_SIZE CONFIG_DYNAMIC_THREAD_STACK_SIZE
#else
#define DYNAMIC_STACK_SIZE 0
#endif

#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 0
#endif

BUILD_ASSERT(DYNAMIC_STACK_SIZE <= PTHREAD_STACK_MAX);
BUILD_ASSERT(DYNAMIC_STACK_SIZE >= PTHREAD_STACK_MIN);

/* only 2 bits in struct posix_thread_attr for schedpolicy */
BUILD_ASSERT(SCHED_OTHER < BIT(2) && SCHED_FIFO < BIT(2) && SCHED_RR < BIT(2));

BUILD_ASSERT((PTHREAD_CREATE_DETACHED == 0 || PTHREAD_CREATE_JOINABLE == 0) &&
	     (PTHREAD_CREATE_DETACHED == 1 || PTHREAD_CREATE_JOINABLE == 1));

BUILD_ASSERT((PTHREAD_CANCEL_ENABLE == 0 || PTHREAD_CANCEL_DISABLE == 0) &&
	     (PTHREAD_CANCEL_ENABLE == 1 || PTHREAD_CANCEL_DISABLE == 1));

BUILD_ASSERT(CONFIG_POSIX_PTHREAD_ATTR_STACKSIZE_BITS + CONFIG_POSIX_PTHREAD_ATTR_GUARDSIZE_BITS <=
	     32);

static inline size_t posix_thread_attr_get_stacksize(const struct posix_thread_attr *attr)
{
	return attr->stacksize + 1;
}

static inline void posix_thread_attr_set_stacksize(struct posix_thread_attr *attr, size_t stacksize)
{
	attr->stacksize = stacksize - 1;
}

static int pthread_concurrency;

pthread_t pthread_self(void)
{
	return (pthread_t)(uintptr_t)k_current_get();
}

int pthread_equal(pthread_t pt1, pthread_t pt2)
{
	return (int)(pt1 == pt2);
}

static bool is_posix_policy_prio_valid(int priority, int policy)
{
	if ((priority >= posix_sched_priority_min(policy)) &&
	    (priority <= posix_sched_priority_max(policy))) {
		return true;
	}

	LOG_DBG("Invalid priority %d and / or policy %d", priority, policy);

	return false;
}

/* Non-static so that they can be tested in ztest */
int zephyr_to_posix_priority(int z_prio, int *policy)
{
	int priority;

	if (z_prio < 0) {
		__ASSERT_NO_MSG(-z_prio <= CONFIG_NUM_COOP_PRIORITIES);
	} else {
		__ASSERT_NO_MSG(z_prio < CONFIG_NUM_PREEMPT_PRIORITIES);
	}

	*policy = (z_prio < 0) ? SCHED_FIFO : SCHED_RR;
	priority = ZEPHYR_TO_POSIX_PRIORITY(z_prio);
	__ASSERT_NO_MSG(is_posix_policy_prio_valid(priority, *policy));

	return priority;
}

/* Non-static so that they can be tested in ztest */
int posix_to_zephyr_priority(int priority, int policy)
{
	__ASSERT_NO_MSG(is_posix_policy_prio_valid(priority, policy));

	return POSIX_TO_ZEPHYR_PRIORITY(priority, policy);
}

static bool __attr_is_runnable(const struct posix_thread_attr *attr)
{
	size_t stacksize;

	if (attr == NULL || attr->stack == NULL) {
		LOG_DBG("attr %p is not initialized", attr);
		return false;
	}

	stacksize = posix_thread_attr_get_stacksize(attr);
	if (stacksize < PTHREAD_STACK_MIN) {
		return false;
	}

	/* require a valid scheduler policy */
	if (!valid_posix_policy(attr->schedpolicy)) {
		return false;
	}

	return true;
}

static bool __attr_is_initialized(const struct posix_thread_attr *attr)
{
	if (IS_ENABLED(CONFIG_DYNAMIC_THREAD)) {
		return __attr_is_runnable(attr);
	}

	if (attr == NULL || !attr->initialized) {
		return false;
	}

	return true;
}

int pthread_attr_setschedparam(pthread_attr_t *_attr, const struct sched_param *schedparam)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || schedparam == NULL ||
	    !is_posix_policy_prio_valid(schedparam->sched_priority, attr->schedpolicy)) {
		return EINVAL;
	}

	attr->priority = schedparam->sched_priority;
	return 0;

	return ENOSYS;
}

int pthread_attr_setstack(pthread_attr_t *_attr, void *stackaddr, size_t stacksize)
{
	int ret;
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (stackaddr == NULL) {
		return EACCES;
	}

	if (!__attr_is_initialized(attr) || (stacksize == 0) || (stacksize < PTHREAD_STACK_MIN) ||
	    (stacksize > PTHREAD_STACK_MAX)) {
		return EINVAL;
	}

	if (attr->stack != NULL) {
		(void)k_thread_stack_free(attr->stack);
	}

	attr->stack = stackaddr;
	posix_thread_attr_set_stacksize(attr, stacksize);

	return 0;
}

int pthread_attr_getscope(const pthread_attr_t *_attr, int *contentionscope)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || contentionscope == NULL) {
		return EINVAL;
	}
	*contentionscope = attr->contentionscope;
	return 0;
}

int pthread_attr_setscope(pthread_attr_t *_attr, int contentionscope)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr)) {
		LOG_DBG("attr %p is not initialized", attr);
		return EINVAL;
	}
	if (!(contentionscope == PTHREAD_SCOPE_PROCESS ||
	      contentionscope == PTHREAD_SCOPE_SYSTEM)) {
		return EINVAL;
	}
	if (contentionscope == PTHREAD_SCOPE_PROCESS) {
		return ENOTSUP;
	}
	attr->contentionscope = contentionscope;
	return 0;
}

int pthread_attr_getinheritsched(const pthread_attr_t *_attr, int *inheritsched)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || inheritsched == NULL) {
		return EINVAL;
	}
	*inheritsched = attr->inheritsched;
	return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *_attr, int inheritsched)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr)) {
		return EINVAL;
	}

	if (inheritsched != PTHREAD_INHERIT_SCHED && inheritsched != PTHREAD_EXPLICIT_SCHED) {
		return EINVAL;
	}

	attr->inheritsched = inheritsched;
	return 0;
}

FUNC_NORETURN
static void zephyr_thread_wrapper(void *arg1, void *arg2, void *arg3)
{
	void *(*fun_ptr)(void *arg) = arg2;
	void *retval = fun_ptr(arg1);

	CODE_UNREACHABLE;
}

int pthread_create(pthread_t *ZRESTRICT thread, const pthread_attr_t *ZRESTRICT attr,
		   void *(*start_routine)(void *), void *ZRESTRICT arg)
{
	struct posix_thread_attr *attrp;
	struct posix_thread_attr default_attr;

	if (attr == NULL) {
		attrp = &default_attrp;

		int ret = pthread_attr_init((pthread_attr_t *)attrp);

		if (ret != 0) {
			return ret;
		}
	} else {
		attrp = (struct posix_thread_attr *)attr;

		if (!attrp->initialized) {
			return EINVAL;
		}
	}

	return -sys_thread_create((struct kthread **)th, attrp->stack, attrp->stacksize,
				  zephyr_thread_wrapper, arg, start_routine, NULL, attrp->priority,
				  k_is_user_context() ? K_USER : 0);
}

int pthread_getconcurrency(void)
{
	return (int)atomic_get(&pthread_concurrency);
}

int pthread_setconcurrency(int new_level)
{
	if (new_level < 0) {
		return EINVAL;
	}

	if (new_level > CONFIG_MP_MAX_NUM_CPUS) {
		return EAGAIN;
	}

	(void)atomic_set(&pthread_concurrency, value);

	return 0;
}

int pthread_setcancelstate(int state, int *oldstate)
{
#if 0
	int ret = EINVAL;
	bool cancel_pending = false;
	struct posix_thread *t = NULL;
	bool cancel_type = -1;

	if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&pthread_pool_lock) {
		t = posix_get_pool_obj_unlocked(&posix_thread_pool, pthread_self());
		if (t == NULL) {
			ret = EINVAL;
			SYS_SEM_LOCK_BREAK;
		}

		if (oldstate != NULL) {
			*oldstate = t->attr.cancelstate;
		}

		t->attr.cancelstate = state;
		cancel_pending = t->attr.cancelpending;
		cancel_type = t->attr.canceltype;

		ret = 0;
	}

	if (ret == 0 && state == PTHREAD_CANCEL_ENABLE &&
	    cancel_type == PTHREAD_CANCEL_ASYNCHRONOUS && cancel_pending) {
		sys_thread_finalize(&t->thread, PTHREAD_CANCELED);
	}

	return ret;
#endif
	return -ENOSYS;
}

int pthread_setcanceltype(int type, int *oldtype)
{
#if 0
	int ret = EINVAL;
	struct posix_thread *t;

	if (type != PTHREAD_CANCEL_DEFERRED && type != PTHREAD_CANCEL_ASYNCHRONOUS) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&pthread_pool_lock) {
		t = posix_get_pool_obj_unlocked(&posix_thread_pool, pthread_self());
		if (t == NULL) {
			ret = EINVAL;
			SYS_SEM_LOCK_BREAK;
		}

		if (oldtype != NULL) {
			*oldtype = t->attr.canceltype;
		}
		t->attr.canceltype = type;

		ret = 0;
	}

	return ret;
#endif
	return -ENOSYS;
}

void pthread_testcancel(void)
{
#if 0
	bool cancel_pended = false;
	struct posix_thread *t = NULL;

	SYS_SEM_LOCK(&pthread_pool_lock) {
		t = posix_get_pool_obj_unlocked(&posix_thread_pool, pthread_self());
		if (t == NULL) {
			SYS_SEM_LOCK_BREAK;
		}
		if (t->attr.cancelstate != PTHREAD_CANCEL_ENABLE) {
			SYS_SEM_LOCK_BREAK;
		}
		if (t->attr.cancelpending) {
			cancel_pended = true;
			t->attr.cancelstate = PTHREAD_CANCEL_DISABLE;
		}
	}

	if (cancel_pended) {
		sys_thread_finalize(&t->thread, PTHREAD_CANCELED);
	}
#endif
}

int pthread_cancel(pthread_t pthread)
{
#if 0
	int ret = ESRCH;
	bool cancel_state = PTHREAD_CANCEL_ENABLE;
	bool cancel_type = PTHREAD_CANCEL_DEFERRED;
	struct posix_thread *t = NULL;

	SYS_SEM_LOCK(&pthread_pool_lock) {
		t = posix_get_pool_obj_unlocked(&posix_thread_pool, pthread);
		if (t == NULL) {
			ret = ESRCH;
			SYS_SEM_LOCK_BREAK;
		}

		if (!__attr_is_initialized(&t->attr)) {
			/* thread has already terminated */
			ret = ESRCH;
			SYS_SEM_LOCK_BREAK;
		}

		ret = 0;
		t->attr.cancelpending = true;
		cancel_state = t->attr.cancelstate;
		cancel_type = t->attr.canceltype;
	}

	if (ret == 0 && cancel_state == PTHREAD_CANCEL_ENABLE &&
	    cancel_type == PTHREAD_CANCEL_ASYNCHRONOUS) {
		sys_thread_finalize(&t->thread, PTHREAD_CANCELED);
	}

	return ret;
#endif
	return ENOSYS;
}

int pthread_setschedparam(pthread_t pthread, int policy, const struct sched_param *param)
{
#if 0
	int ret = ESRCH;
	int new_prio = K_LOWEST_APPLICATION_THREAD_PRIO;
	struct posix_thread *t = NULL;

	if (param == NULL || !valid_posix_policy(policy) ||
	    !is_posix_policy_prio_valid(param->sched_priority, policy)) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&pthread_pool_lock) {
		t = posix_get_pool_obj_unlocked(&posix_thread_pool, pthread);
		if (t == NULL) {
			ret = ESRCH;
			SYS_SEM_LOCK_BREAK;
		}

		ret = 0;
		new_prio = posix_to_zephyr_priority(param->sched_priority, policy);
	}

	if (ret == 0) {
		k_thread_priority_set(&t->thread, new_prio);
	}

	return ret;
#endif
	return ENOSYS;
}

int pthread_setschedprio(pthread_t thread, int prio)
{
#if 0
	int ret;
	int new_prio = K_LOWEST_APPLICATION_THREAD_PRIO;
	struct posix_thread *t = NULL;
	int policy = -1;
	struct sched_param param;

	ret = pthread_getschedparam(thread, &policy, &param);
	if (ret != 0) {
		return ret;
	}

	if (!is_posix_policy_prio_valid(prio, policy)) {
		return EINVAL;
	}

	ret = ESRCH;
	SYS_SEM_LOCK(&pthread_pool_lock) {
		t = posix_get_pool_obj_unlocked(&posix_thread_pool, thread);
		if (t == NULL) {
			ret = ESRCH;
			SYS_SEM_LOCK_BREAK;
		}

		ret = 0;
		new_prio = posix_to_zephyr_priority(prio, policy);
	}

	if (ret == 0) {
		k_thread_priority_set(&t->thread, new_prio);
	}

	return ret;
#endif

	return ENOSYS;
}

int pthread_attr_init(pthread_attr_t *_attr)
{
	struct posix_thread_attr *const attr = (struct posix_thread_attr *)_attr;

	if (attr == NULL) {
		return ENOMEM;
	}

	*attr = (struct posix_thread_attr){
		.guardsize = CONFIG_POSIX_PTHREAD_ATTR_GUARDSIZE_DEFAULT,
		.contentionscope = PTHREAD_SCOPE_SYSTEM,
		.inheritsched = PTHREAD_INHERIT_SCHED,
		.detachstate = PTHREAD_CREATE_JOINABLE,
		.cancelstate = PTHREAD_CANCEL_ENABLE,
		.canceltype = PTHREAD_CANCEL_DEFERRED,
		.priority = DEFAULT_PTHREAD_PRIORITY,
		.schedpolicy = DEFAULT_PTHREAD_POLICY,
		.stack = NULL,
		.stacksize = 0,
		.initialized = false,
		.caller_destroys = true,
	};

	if (DYNAMIC_STACK_SIZE > 0) {
		attr->stack = k_thread_stack_alloc(DYNAMIC_STACK_SIZE + attr->guardsize,
						   k_is_user_context() ? K_USER : 0);
		if (attr->stack == NULL) {
		} else {
			posix_thread_attr_set_stacksize(attr, DYNAMIC_STACK_SIZE);
			__ASSERT_NO_MSG(__attr_is_initialized(attr));
			LOG_DBG("Allocated thread stack %zu@%p",
				posix_thread_attr_get_stacksize(attr), attr->stack);
		}
	}

	/* caller responsible for destroying attr */
	attr->initialized = true;

	LOG_DBG("Initialized attr %p", _attr);

	return 0;
}

int pthread_getschedparam(pthread_t pthread, int *policy, struct sched_param *param)
{
#if 0
	int ret = ESRCH;
	struct posix_thread *t;

	if (policy == NULL || param == NULL) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&pthread_pool_lock) {
		t = posix_get_pool_obj_unlocked(&posix_thread_pool, pthread);
		if (t == NULL) {
			ret = ESRCH;
			SYS_SEM_LOCK_BREAK;
		}

		if (!__attr_is_initialized(&t->attr)) {
			ret = ESRCH;
			SYS_SEM_LOCK_BREAK;
		}

		ret = 0;
		param->sched_priority =
			zephyr_to_posix_priority(k_thread_priority_get(&t->thread), policy);
	}

	return ret;
#endif
	return ENOSYS;
}

int pthread_once(pthread_once_t *once, void (*init_func)(void))
{
	sys_thread_once((sys_thread_once_t *)once, init_func);

	return 0;
}

FUNC_NORETURN
void pthread_exit(void *retval)
{
	sys_thread_exit(retval);
	CODE_UNREACHABLE;
}

int pthread_timedjoin_np(pthread_t pthread, void **status, const struct timespec *abstime)
{
	return sys_thread_rejoin((struct k_thread *)pthread, status,
				 (abstime == NULL)
					 ? K_FOREVER
					 : sys_timepoint_timeout(timespec_to_timepoint(abstime)));
}

int pthread_tryjoin_np(pthread_t pthread, void **status)
{
	return sys_thread_rejoin((struct k_thread *)pthread, status, K_NO_WAIT);
}

int pthread_join(pthread_t pthread, void **status)
{
	return sys_thread_rejoin((struct k_thread *)pthread, status, K_FOREVER);
}

int pthread_detach(pthread_t pthread)
{
	return sys_thread_detach((struct k_thread *)pthread);
}

int pthread_attr_getdetachstate(const pthread_attr_t *_attr, int *detachstate)
{
	const struct posix_thread_attr *attr = (const struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || (detachstate == NULL)) {
		return EINVAL;
	}

	*detachstate = attr->detachstate;
	return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *_attr, int detachstate)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || ((detachstate != PTHREAD_CREATE_DETACHED) &&
					     (detachstate != PTHREAD_CREATE_JOINABLE))) {
		return EINVAL;
	}

	attr->detachstate = detachstate;
	return 0;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *_attr, int *policy)
{
	const struct posix_thread_attr *attr = (const struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || (policy == NULL)) {
		return EINVAL;
	}

	*policy = attr->schedpolicy;
	return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *_attr, int policy)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || !valid_posix_policy(policy)) {
		return EINVAL;
	}

	attr->schedpolicy = policy;
	return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *_attr, size_t *stacksize)
{
	const struct posix_thread_attr *attr = (const struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || (stacksize == NULL)) {
		return EINVAL;
	}

	*stacksize = posix_thread_attr_get_stacksize(attr);
	return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *_attr, size_t stacksize)
{
	int ret;
	void *new_stack;
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || stacksize == 0 || stacksize < PTHREAD_STACK_MIN ||
	    stacksize > PTHREAD_STACK_MAX) {
		return EINVAL;
	}

	if (posix_thread_attr_get_stacksize(attr) == stacksize) {
		return 0;
	}

	new_stack =
		k_thread_stack_alloc(stacksize + attr->guardsize, k_is_user_context() ? K_USER : 0);
	if (new_stack == NULL) {
		if (stacksize < posix_thread_attr_get_stacksize(attr)) {
			posix_thread_attr_set_stacksize(attr, stacksize);
			return 0;
		}

		return ENOMEM;
	}

	if (attr->stack != NULL) {
		ret = k_thread_stack_free(attr->stack);
	}

	posix_thread_attr_set_stacksize(attr, stacksize);
	attr->stack = new_stack;

	return 0;
}

/**
 * @brief Get stack attributes in thread attributes object.
 *
 * See IEEE 1003.1
 */
int pthread_attr_getstack(const pthread_attr_t *_attr, void **stackaddr, size_t *stacksize)
{
	const struct posix_thread_attr *attr = (const struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || (stackaddr == NULL) || (stacksize == NULL)) {
		return EINVAL;
	}

	*stackaddr = attr->stack;
	*stacksize = posix_thread_attr_get_stacksize(attr);
	return 0;
}

int pthread_attr_getguardsize(const pthread_attr_t *ZRESTRICT _attr, size_t *ZRESTRICT guardsize)
{
	struct posix_thread_attr *const attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || guardsize == NULL) {
		return EINVAL;
	}

	*guardsize = attr->guardsize;

	return 0;
}

int pthread_attr_setguardsize(pthread_attr_t *_attr, size_t guardsize)
{
	struct posix_thread_attr *const attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || guardsize > PTHREAD_GUARD_MAX) {
		return EINVAL;
	}

	attr->guardsize = guardsize;

	return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *_attr, struct sched_param *schedparam)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr) || (schedparam == NULL)) {
		return EINVAL;
	}

	schedparam->sched_priority = attr->priority;
	return 0;
}

/**
 * @brief Destroy thread attributes object.
 *
 * See IEEE 1003.1
 */
int pthread_attr_destroy(pthread_attr_t *_attr)
{
	int ret;
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!__attr_is_initialized(attr)) {
		return EINVAL;
	}

	ret = k_thread_stack_free(attr->stack);
	if (ret == 0) {
		LOG_DBG("Freed attr %p thread stack %zu@%p", _attr,
			posix_thread_attr_get_stacksize(attr), attr->stack);
	}

	*attr = (struct posix_thread_attr){0};
	LOG_DBG("Destroyed attr %p", _attr);

	return 0;
}

int pthread_setname_np(pthread_t thread, const char *name)
{
#ifdef CONFIG_THREAD_NAME
	k_tid_t kthread;
	struct posix_thread *t;

	t = posix_get_pool_obj(&posix_thread_pool, &pthread_pool_lock, thread);
	if (t == NULL) {
		return ESRCH;
	}

	kthread = &t->thread;

	if (name == NULL) {
		return EINVAL;
	}

	return k_thread_name_set(kthread, name);
#else
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
	return 0;
#endif
}

int pthread_getname_np(pthread_t thread, char *name, size_t len)
{
#ifdef CONFIG_THREAD_NAME
	return k_thread_name_copy((struct k_thread *)(uintptr_t)thread, name, len - 1);
#else
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
	ARG_UNUSED(len);
	return 0;
#endif
}

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
	ARG_UNUSED(prepare);
	ARG_UNUSED(parent);
	ARG_UNUSED(child);

	return ENOSYS;
}

/* this should probably go into signal.c but we need access to the lock */
int pthread_sigmask(int how, const sigset_t *ZRESTRICT set, sigset_t *ZRESTRICT oset)
{
	int ret = ESRCH;
	struct posix_thread *t = NULL;

	if (!(how == SIG_BLOCK || how == SIG_SETMASK || how == SIG_UNBLOCK)) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&pthread_pool_lock) {
		t = posix_get_pool_obj_unlocked(&posix_thread_pool, pthread_self());
		if (t == NULL) {
			ret = ESRCH;
			SYS_SEM_LOCK_BREAK;
		}

		if (oset != NULL) {
			*oset = t->sigset;
		}

		ret = 0;
		if (set == NULL) {
			SYS_SEM_LOCK_BREAK;
		}

		const unsigned long *const x = (const unsigned long *)set;
		unsigned long *const y = (unsigned long *)&t->sigset;

		switch (how) {
		case SIG_BLOCK:
			for (size_t i = 0; i < sizeof(sigset_t) / sizeof(unsigned long); ++i) {
				y[i] |= x[i];
			}
			break;
		case SIG_SETMASK:
			t->sigset = *set;
			break;
		case SIG_UNBLOCK:
			for (size_t i = 0; i < sizeof(sigset_t) / sizeof(unsigned long); ++i) {
				y[i] &= ~x[i];
			}
			break;
		}
	}

	return ret;
}

int sched_yield(void)
{
	k_yield();
	return 0;
}

struct posix_thread *to_posix_thread(pthread_t pthread)
{
	return posix_get_pool_obj(&posix_thread_pool, &pthread_pool_lock, pthread);
}
