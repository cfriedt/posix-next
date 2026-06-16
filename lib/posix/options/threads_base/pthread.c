/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_clock.h"
#include "posix_internal.h"

#include <limits.h>
#include <pthread.h>
#include <stdalign.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/elastipool.h>
#include <zephyr/sys/sem.h>
#include <zephyr/sys/slist.h>
#include <zephyr/sys/thread.h>
#include <zephyr/sys/util.h>

#define DEFAULT_PTHREAD_POLICY (IS_ENABLED(CONFIG_PREEMPT_ENABLED) ? SCHED_RR : SCHED_FIFO)

/* only 2 bits in struct posix_thread_attr for schedpolicy */
BUILD_ASSERT(SCHED_OTHER < BIT(2) && SCHED_FIFO < BIT(2) && SCHED_RR < BIT(2));

BUILD_ASSERT((PTHREAD_CREATE_DETACHED == 0 || PTHREAD_CREATE_JOINABLE == 0) &&
	     (PTHREAD_CREATE_DETACHED == 1 || PTHREAD_CREATE_JOINABLE == 1));

BUILD_ASSERT((PTHREAD_CANCEL_ENABLE == 0 || PTHREAD_CANCEL_DISABLE == 0) &&
	     (PTHREAD_CANCEL_ENABLE == 1 || PTHREAD_CANCEL_DISABLE == 1));

LOG_MODULE_REGISTER(pthread, CONFIG_PTHREAD_LOG_LEVEL);

static inline void posix_thread_attr_init(struct posix_thread_attr *attr)
{
	*attr = (struct posix_thread_attr){
		.stack = NULL,
		.stacksize = 0,
		.guardsize = 0,
		.priority = posix_thread_attr_default_priority(),
		.schedpolicy = DEFAULT_PTHREAD_POLICY,
		.cancelstate = PTHREAD_CANCEL_ENABLE,
		.canceltype = PTHREAD_CANCEL_DEFERRED,
		.contentionscope = PTHREAD_SCOPE_SYSTEM,
		.detachstate = PTHREAD_CREATE_JOINABLE,
		.inheritsched = PTHREAD_INHERIT_SCHED,
		.initialized = true,
	};
}

pthread_t pthread_self(void)
{
	return to_pthread_thread(k_current_get());
}

int pthread_equal(pthread_t pt1, pthread_t pt2)
{
	return (int)(pt1 == pt2);
}

int pthread_attr_setschedparam(pthread_attr_t *_attr, const struct sched_param *schedparam)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || (schedparam == NULL) ||
	    !is_posix_policy_prio_valid(schedparam->sched_priority, attr->schedpolicy)) {
		return EINVAL;
	}

	attr->priority = schedparam->sched_priority;
	return 0;
}

FUNC_NORETURN
static void zephyr_thread_wrapper(void *arg1, void *arg2, void *arg3)
{
	void *(*fun_ptr)(void *arg) = arg2;

	k_thread_exit(fun_ptr(arg1));
	CODE_UNREACHABLE;
}

int pthread_create(pthread_t *ZRESTRICT thread, const pthread_attr_t *ZRESTRICT attr,
		   void *(*start_routine)(void *), void *ZRESTRICT arg)
{
	int ret;
	int prio;
	uint32_t options = 0;
	struct k_thread *k_thread;
	struct posix_thread_attr *attrp;
	struct posix_thread_attr default_attr __aligned(alignof(pthread_attr_t));

	if (attr == NULL) {
		attrp = &default_attr;
		posix_thread_attr_init(attrp);
		__ASSERT_NO_MSG(posix_thread_attr_is_valid(attrp));
	} else {
		attrp = (struct posix_thread_attr *)attr;
		if (!posix_thread_attr_is_valid(attrp)) {
			return EINVAL;
		}
	}

	if (attrp->detachstate == PTHREAD_CREATE_DETACHED) {
		options |= K_DETACHED;
	}

#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
	if (attrp->inheritsched == PTHREAD_INHERIT_SCHED) {
		prio = k_thread_priority_get(k_current_get());
	} else
#endif /* _POSIX_THREAD_PRIORITY_SCHEDULING */
	{
		prio = posix_to_zephyr_priority(attrp->priority, attrp->schedpolicy);
	}

	ret = -sys_thread_create(&k_thread, attrp->stack, attrp->stacksize, attrp->guardsize,
				 zephyr_thread_wrapper, arg, start_routine, NULL, prio, options);

	if (ret == 0) {
		*thread = to_pthread_thread(k_thread);
	}

	return ret;
}

int pthread_setcancelstate(int state, int *oldstate)
{
	bool enabled;

	if (oldstate != NULL) {
		enabled = k_thread_cancel_getstate();
		*oldstate = enabled ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE;
	}

	switch (state) {
	case PTHREAD_CANCEL_ENABLE:
		enabled = true;
		break;
	case PTHREAD_CANCEL_DISABLE:
		enabled = false;
		break;
	default:
		return EINVAL;
	}

	k_thread_cancel_setstate(enabled);

	return 0;
}

int pthread_setcanceltype(int type, int *oldtype)
{
	const bool async = true;
	bool n_type;
	bool o_type;

	switch (type) {
	case PTHREAD_CANCEL_ASYNCHRONOUS:
		n_type = async;
		break;
	case PTHREAD_CANCEL_DEFERRED:
		n_type = !async;
		break;
	default:
		return EINVAL;
	}

	k_thread_cancel_qtype(&n_type, &o_type);

	if (oldtype != NULL) {
		*oldtype =
			(o_type == async) ? PTHREAD_CANCEL_ASYNCHRONOUS : PTHREAD_CANCEL_DEFERRED;
	}

	return 0;
}

void pthread_testcancel(void)
{
	k_thread_testcancel();
}

int pthread_cancel(pthread_t pthread)
{
	return -k_thread_cancel(to_k_thread(&pthread));
}

int pthread_attr_init(pthread_attr_t *attr)
{
	posix_thread_attr_init((struct posix_thread_attr *)attr);
	return 0;
}

int pthread_once(pthread_once_t *once, void (*init_func)(void))
{
	sys_thread_once((sys_thread_once_t *)once, init_func);

	return 0;
}

FUNC_NORETURN
void pthread_exit(void *retval)
{
	k_thread_exit(retval);
	CODE_UNREACHABLE;
}

int pthread_join(pthread_t pthread, void **status)
{
	return -k_thread_rejoin(to_k_thread(&pthread), status, K_FOREVER);
}

int pthread_detach(pthread_t pthread)
{
	return -k_thread_detach(to_k_thread(&pthread));
}

int pthread_attr_getdetachstate(const pthread_attr_t *_attr, int *detachstate)
{
	const struct posix_thread_attr *attr = (const struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || (detachstate == NULL)) {
		return EINVAL;
	}

	*detachstate = attr->detachstate;
	return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *_attr, int detachstate)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || ((detachstate != PTHREAD_CREATE_DETACHED) &&
						  (detachstate != PTHREAD_CREATE_JOINABLE))) {
		return EINVAL;
	}

	attr->detachstate = detachstate;
	return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *_attr, struct sched_param *schedparam)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || (schedparam == NULL)) {
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
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr)) {
		return EINVAL;
	}

	*attr = (struct posix_thread_attr){0};
	LOG_DBG("Destroyed attr %p", _attr);

	return 0;
}

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
	ARG_UNUSED(prepare);
	ARG_UNUSED(parent);
	ARG_UNUSED(child);

	return ENOSYS;
}

int pthread_sigmask(int how, const sigset_t *ZRESTRICT set, sigset_t *ZRESTRICT oset)
{
	int k_how;

	switch(how) {
	case SIG_BLOCK:
		k_how = K_SIG_BLOCK;
		break;
	case SIG_SETMASK:
		k_how = K_SIG_SETMASK;
		break;
	case SIG_UNBLOCK:
		k_how = K_SIG_UNBLOCK;
		break;
	default:
		return EINVAL;
	}

	struct k_sig_set kset_buf, okset;
	const struct k_sig_set *kset = NULL;
	sigset_t oset_buf;

	if (set != NULL) {
#if defined(CONFIG_SIGNAL) && !IS_ENABLED(CONFIG_NATIVE_LIBC)
		kset = z_sig_set_from_posix(set, &kset_buf);
#else
		memcpy(&kset_buf, set, sizeof(kset_buf));
		kset = &kset_buf;
#endif
	}

	int ret = k_sig_mask(k_how, kset, oset == NULL ? NULL : &okset);
	if (ret < 0) {
		return -ret;
	}

	if (oset != NULL) {
#if defined(CONFIG_SIGNAL) && !IS_ENABLED(CONFIG_NATIVE_LIBC)
		*oset = *z_sig_set_to_posix(&okset, &oset_buf);
#else
		memcpy(oset, &okset, sizeof(*oset));
#endif
	}

	return 0;
}

int pthread_kill(pthread_t thread, int sig)
{
	return -k_sig_queue(to_k_thread(&thread), sig, (union k_sig_val){0});
}

int sched_yield(void)
{
	k_yield();
	return 0;
}
