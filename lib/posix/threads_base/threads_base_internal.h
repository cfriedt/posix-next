/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_OPTIONS_THREADS_BASE_THREADS_BASE_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_OPTIONS_THREADS_BASE_THREADS_BASE_INTERNAL_H_

#include "posix_internal.h"

#include <pthread.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/thread.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#define DEFAULT_PTHREAD_POLICY (IS_ENABLED(CONFIG_PREEMPT_ENABLED) ? SCHED_RR : SCHED_FIFO)

/* only 3 bits in struct posix_thread_attr for schedpolicy */
BUILD_ASSERT(SCHED_OTHER < BIT(3) && SCHED_FIFO < BIT(3) && SCHED_RR < BIT(3));
#ifdef SCHED_SPORADIC
BUILD_ASSERT(SCHED_SPORADIC < BIT(3));
#endif

BUILD_ASSERT((PTHREAD_CREATE_DETACHED == 0 || PTHREAD_CREATE_JOINABLE == 0) &&
	     (PTHREAD_CREATE_DETACHED == 1 || PTHREAD_CREATE_JOINABLE == 1));

BUILD_ASSERT((PTHREAD_CANCEL_ENABLE == 0 || PTHREAD_CANCEL_DISABLE == 0) &&
	     (PTHREAD_CANCEL_ENABLE == 1 || PTHREAD_CANCEL_DISABLE == 1));

static ALWAYS_INLINE void posix_thread_attr_init(struct posix_thread_attr *attr)
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

#include <errno.h>
#include <limits.h>
#include <stddef.h>

#include <zephyr/sys/atomic.h>
#include <zephyr/sys/timeutil.h>

/*
 * With CONFIG_POSIX_THREAD_FUTEX, pthread_mutex_t and pthread_cond_t are the futex-backed
 * structs; otherwise they are 32-bit handles to pooled kernel objects. Each helper below is
 * written for one layout and only reached in that configuration: the other configuration's
 * layout is behind a cast that is never dereferenced there.
 */
struct k_futex;

#define POSIX_FUTEX_MUTEX(m) ((struct posix_futex_mutex *)(m))
#define POSIX_FUTEX_COND(cv) ((struct posix_futex_cond *)(cv))
#define POSIX_HANDLE(obj)    ((uint32_t *)(obj))

/* struct posix_futex_mutex::val */
#define POSIX_MUTEX_UNLOCKED  0
#define POSIX_MUTEX_LOCKED    1
#define POSIX_MUTEX_CONTENDED 2

/* struct posix_futex_cond::flags */
#define POSIX_COND_CLOCK_MONOTONIC BIT(0)

BUILD_ASSERT(sizeof(pthread_t) <= sizeof(((struct posix_futex_mutex *)0)->owner));
BUILD_ASSERT(sizeof(pthread_mutex_t) >= sizeof(uint32_t));
BUILD_ASSERT(sizeof(pthread_cond_t) >= sizeof(uint32_t));

/*
 * k_futex has no priority inheritance, so PTHREAD_PRIO_INHERIT mutexes stay on k_mutex. The
 * protocol field is only ever set through pthread_mutexattr_setprotocol(), which admits
 * PTHREAD_PRIO_INHERIT alone besides PTHREAD_PRIO_NONE.
 */
static ALWAYS_INLINE bool posix_mutex_is_pi(const pthread_mutex_t *m)
{
	return IS_ENABLED(CONFIG_POSIX_THREAD_FUTEX) && (POSIX_FUTEX_MUTEX(m)->protocol != 0);
}

/* all-zero futex objects are valid, so only handles need a first-use init */
static ALWAYS_INLINE bool posix_mutex_is_static_init(const pthread_mutex_t *m)
{
	return !IS_ENABLED(CONFIG_POSIX_THREAD_FUTEX) &&
	       (*POSIX_HANDLE(m) == (uint32_t)_PTHREAD_HANDLE_INITIALIZER);
}

static ALWAYS_INLINE bool posix_cond_is_static_init(const pthread_cond_t *cv)
{
	return !IS_ENABLED(CONFIG_POSIX_THREAD_FUTEX) &&
	       (*POSIX_HANDLE(cv) == (uint32_t)_PTHREAD_HANDLE_INITIALIZER);
}

static ALWAYS_INLINE void posix_mutex_set_handle(pthread_mutex_t *m, struct k_mutex *km)
{
	if (!IS_ENABLED(CONFIG_POSIX_THREAD_FUTEX)) {
		*POSIX_HANDLE(m) = (uint32_t)(uintptr_t)km;
	}
}

static ALWAYS_INLINE void posix_cond_set_handle(pthread_cond_t *cv, struct k_condvar *kcv)
{
	if (!IS_ENABLED(CONFIG_POSIX_THREAD_FUTEX)) {
		*POSIX_HANDLE(cv) = (uint32_t)(uintptr_t)kcv;
	}
}

static ALWAYS_INLINE int posix_futex_mutex_init(pthread_mutex_t *m,
						const struct pthread_mutexattr *a, int flags)
{
	int ret;
	struct k_mutex *mutex;
	struct posix_futex_mutex *const fm = POSIX_FUTEX_MUTEX(m);

	*fm = (struct posix_futex_mutex){0};

	if (a != NULL) {
		fm->type = (a->type == PTHREAD_MUTEX_DEFAULT) ? PTHREAD_MUTEX_NORMAL : a->type;
		fm->protocol = a->protocol;
	}

	if (posix_mutex_is_pi(m)) {
		ret = sys_mutex_alloc(&mutex, flags);
		if (ret < 0) {
			*fm = (struct posix_futex_mutex){0};
			return -ret;
		}

		fm->val = (atomic_val_t)(uintptr_t)mutex;
	}

	return 0;
}

static ALWAYS_INLINE int posix_futex_mutex_destroy(pthread_mutex_t *m)
{
	int ret;
	struct posix_futex_mutex *const fm = POSIX_FUTEX_MUTEX(m);

	if (posix_mutex_is_pi(m)) {
		ret = sys_mutex_destroy(to_k_mutex(m));
		if (ret < 0) {
			return -ret;
		}
	} else if (atomic_get(&fm->val) != POSIX_MUTEX_UNLOCKED) {
		return EBUSY;
	}

	*fm = (struct posix_futex_mutex){0};

	return 0;
}

static ALWAYS_INLINE void posix_futex_cond_init(pthread_cond_t *cv, uint32_t sys_clock_id)
{
	*POSIX_FUTEX_COND(cv) = (struct posix_futex_cond){
		.flags = (sys_clock_id == SYS_CLOCK_MONOTONIC) ? POSIX_COND_CLOCK_MONOTONIC : 0,
	};
}

static ALWAYS_INLINE int posix_futex_cond_destroy(pthread_cond_t *cv)
{
	struct posix_futex_cond *const fc = POSIX_FUTEX_COND(cv);

	/* woken waiters are already uncounted; a timed-out one uncounts itself before returning */
	while (atomic_get(&fc->waiters) != 0) {
		k_sleep(K_TICKS(1));
	}

	*fc = (struct posix_futex_cond){0};

	return 0;
}

#ifdef CONFIG_USERSPACE
/* the leading words of both objects are a struct k_futex; the kernel maintains ::waiters */
BUILD_ASSERT(offsetof(struct posix_futex_mutex, val) == offsetof(struct k_futex, val));
BUILD_ASSERT(offsetof(struct posix_futex_mutex, waiters) == offsetof(struct k_futex, waiters));
BUILD_ASSERT(offsetof(struct posix_futex_cond, seq) == offsetof(struct k_futex, val));
BUILD_ASSERT(offsetof(struct posix_futex_cond, waiters) == offsetof(struct k_futex, waiters));

#endif /* CONFIG_USERSPACE */

/* k_futex is declared only with userspace, which CONFIG_POSIX_THREAD_FUTEX requires */
static ALWAYS_INLINE int posix_futex_wait(struct k_futex *futex, int expected, k_timeout_t timeout)
{
	ARG_UNUSED(futex);
	ARG_UNUSED(expected);
	ARG_UNUSED(timeout);

	return COND_CODE_1(CONFIG_USERSPACE, (k_futex_wait(futex, expected, timeout)), (-ENOSYS));
}

static ALWAYS_INLINE int posix_futex_wake(struct k_futex *futex, bool wake_all)
{
	ARG_UNUSED(futex);
	ARG_UNUSED(wake_all);

	return COND_CODE_1(CONFIG_USERSPACE, (k_futex_wake(futex, wake_all)), (-ENOSYS));
}

static ALWAYS_INLINE struct k_futex *posix_mutex_futex(pthread_mutex_t *m)
{
	return (struct k_futex *)&POSIX_FUTEX_MUTEX(m)->val;
}

static ALWAYS_INLINE struct k_futex *posix_cond_futex(pthread_cond_t *cv)
{
	return (struct k_futex *)&POSIX_FUTEX_COND(cv)->seq;
}

static ALWAYS_INLINE int posix_futex_mutex_lock(pthread_mutex_t *m, k_timeout_t timeout)
{
	int ret;
	atomic_val_t val;
	pthread_t self = 0;
	struct posix_futex_mutex *const fm = POSIX_FUTEX_MUTEX(m);

	if (fm->type != PTHREAD_MUTEX_NORMAL) {
		self = pthread_self();
		if ((atomic_get(&fm->val) != POSIX_MUTEX_UNLOCKED) && (fm->owner == self)) {
			if (fm->type == PTHREAD_MUTEX_ERRORCHECK) {
				return EDEADLK;
			}
			if (fm->count == UINT16_MAX) {
				return EAGAIN;
			}
			fm->count++;
			return 0;
		}
	}

	if (!atomic_cas(&fm->val, POSIX_MUTEX_UNLOCKED, POSIX_MUTEX_LOCKED)) {
		if (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			return EBUSY;
		}

		val = atomic_set(&fm->val, POSIX_MUTEX_CONTENDED);
		while (val != POSIX_MUTEX_UNLOCKED) {
			ret = posix_futex_wait(posix_mutex_futex(m), POSIX_MUTEX_CONTENDED,
					       timeout);
			if (ret == -ETIMEDOUT) {
				return ETIMEDOUT;
			}
			if ((ret < 0) && (ret != -EAGAIN)) {
				return EINVAL;
			}
			val = atomic_set(&fm->val, POSIX_MUTEX_CONTENDED);
		}
	}

	if (fm->type != PTHREAD_MUTEX_NORMAL) {
		fm->owner = self;
		fm->count = 0;
	}

	return 0;
}

static ALWAYS_INLINE int posix_futex_mutex_unlock(pthread_mutex_t *m)
{
	struct posix_futex_mutex *const fm = POSIX_FUTEX_MUTEX(m);

	if (atomic_get(&fm->val) == POSIX_MUTEX_UNLOCKED) {
		return EPERM;
	}

	if (fm->type != PTHREAD_MUTEX_NORMAL) {
		if (fm->owner != pthread_self()) {
			return EPERM;
		}
		if (fm->count > 0) {
			fm->count--;
			return 0;
		}
		fm->owner = 0;
	}

	if (atomic_dec(&fm->val) != POSIX_MUTEX_LOCKED) {
		atomic_set(&fm->val, POSIX_MUTEX_UNLOCKED);
		(void)posix_futex_wake(posix_mutex_futex(m), false);
	}

	return 0;
}

/* posix_futex_wait() compares against an int: keep the sequence within its range */
static ALWAYS_INLINE void posix_cond_seq_bump(pthread_cond_t *cv)
{
	if (atomic_inc(&POSIX_FUTEX_COND(cv)->seq) == INT_MAX) {
		(void)atomic_and(&POSIX_FUTEX_COND(cv)->seq, INT_MAX);
	}
}

/*
 * A waiter that has released the mutex but not yet blocked is caught by the
 * sequence compare in posix_futex_wait(); one that has blocked is counted by the
 * kernel. Waking a blocked waiter without moving the sequence keeps a signal
 * to one thread; the sequence only moves when no blocked waiter was found.
 */
static ALWAYS_INLINE int posix_futex_cond_signal(pthread_cond_t *cv)
{
	struct posix_futex_cond *const fc = POSIX_FUTEX_COND(cv);

	if ((atomic_get(&fc->waiters) != 0) &&
	    (posix_futex_wake(posix_cond_futex(cv), false) > 0)) {
		return 0;
	}

	posix_cond_seq_bump(cv);
	if (atomic_get(&fc->waiters) != 0) {
		(void)posix_futex_wake(posix_cond_futex(cv), false);
	}

	return 0;
}

static ALWAYS_INLINE int posix_futex_cond_broadcast(pthread_cond_t *cv)
{
	posix_cond_seq_bump(cv);
	if (atomic_get(&POSIX_FUTEX_COND(cv)->waiters) != 0) {
		(void)posix_futex_wake(posix_cond_futex(cv), true);
	}

	return 0;
}

static ALWAYS_INLINE int posix_futex_cond_wait(pthread_cond_t *cv, pthread_mutex_t *m,
					       k_timeout_t timeout)
{
	int ret;
	atomic_val_t seq;

	/* cancellation point, taken while the mutex is still held */
	pthread_testcancel();

	seq = atomic_get(&POSIX_FUTEX_COND(cv)->seq);

	ret = pthread_mutex_unlock(m);
	if (ret != 0) {
		return ret;
	}

	ret = posix_futex_wait(posix_cond_futex(cv), (int)seq, timeout);
	(void)pthread_mutex_lock(m);

	return (ret == -ETIMEDOUT) ? ETIMEDOUT : 0;
}

static ALWAYS_INLINE int posix_futex_cond_clockwait(pthread_cond_t *cv, pthread_mutex_t *m,
						    clockid_t clock_id,
						    const struct timespec *abstime)
{
	uint32_t clock;

	if (abstime == NULL) {
		return posix_futex_cond_wait(cv, m, K_FOREVER);
	}

	if (clock_id == -1) {
		clock = ((POSIX_FUTEX_COND(cv)->flags & POSIX_COND_CLOCK_MONOTONIC) != 0)
				? SYS_CLOCK_MONOTONIC
				: SYS_CLOCK_REALTIME;
	} else if (clock_id == CLOCK_REALTIME) {
		clock = SYS_CLOCK_REALTIME;
	} else if (clock_id == CLOCK_MONOTONIC) {
		clock = SYS_CLOCK_MONOTONIC;
	} else {
		return EINVAL;
	}

	if (!timespec_is_valid(abstime)) {
		return EINVAL;
	}

	return posix_futex_cond_wait(cv, m, timespec_abs_to_timeout(clock, abstime));
}

static ALWAYS_INLINE int pthread_mutex_lock_common(pthread_mutex_t *m, k_timeout_t timeout)
{
	int ret;

	if (IS_ENABLED(CONFIG_POSIX_THREAD_FUTEX) && !posix_mutex_is_pi(m)) {
		return posix_futex_mutex_lock(m, timeout);
	}

	if (posix_mutex_is_static_init(m)) {
		ret = pthread_mutex_init(m, NULL);
		if (ret != 0) {
			return ret;
		}
	}

	return -k_mutex_lock(to_k_mutex(m), timeout);
}

static ALWAYS_INLINE int cond_wait(pthread_cond_t *cvar, pthread_mutex_t *mu, clockid_t clock_id,
				   const struct timespec *abstime)
{
	int ret;

	if (IS_ENABLED(CONFIG_POSIX_THREAD_FUTEX)) {
		return posix_futex_cond_clockwait(cvar, mu, clock_id, abstime);
	}

	if (posix_mutex_is_static_init(mu)) {
		ret = pthread_mutex_init(mu, NULL);
		if (ret != 0) {
			return ret;
		}
	}

	if (posix_cond_is_static_init(cvar)) {
		ret = pthread_cond_init(cvar, NULL);
		if (ret != 0) {
			return ret;
		}
	}

	if (abstime == NULL) {
		return -k_condvar_wait(to_k_condvar(cvar), to_k_mutex(mu), K_FOREVER);
	}

	if (clock_id == -1) {
		return -k_condvar_timedwait(to_k_condvar(cvar), to_k_mutex(mu), abstime);
	}

	return -k_condvar_clockwait(to_k_condvar(cvar), to_k_mutex(mu), clock_id, abstime);
}

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_THREADS_BASE_THREADS_BASE_INTERNAL_H_ */
