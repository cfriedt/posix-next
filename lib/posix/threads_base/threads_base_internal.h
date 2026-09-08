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

#ifdef CONFIG_POSIX_THREAD_FUTEX

#include <errno.h>
#include <limits.h>
#include <stddef.h>

#include <zephyr/sys/atomic.h>
#include <zephyr/sys/timeutil.h>

/* pthread_mutex_t::val */
#define POSIX_MUTEX_UNLOCKED  0
#define POSIX_MUTEX_LOCKED    1
#define POSIX_MUTEX_CONTENDED 2

/* pthread_cond_t::flags */
#define POSIX_COND_CLOCK_MONOTONIC BIT(0)

/* the leading words of both objects are a struct k_futex; the kernel maintains ::waiters */
BUILD_ASSERT(offsetof(pthread_mutex_t, val) == offsetof(struct k_futex, val));
BUILD_ASSERT(offsetof(pthread_mutex_t, waiters) == offsetof(struct k_futex, waiters));
BUILD_ASSERT(offsetof(pthread_cond_t, seq) == offsetof(struct k_futex, val));
BUILD_ASSERT(offsetof(pthread_cond_t, waiters) == offsetof(struct k_futex, waiters));
BUILD_ASSERT(sizeof(pthread_t) <= sizeof(((pthread_mutex_t *)0)->owner));

static ALWAYS_INLINE struct k_futex *posix_mutex_futex(pthread_mutex_t *m)
{
	return (struct k_futex *)&m->val;
}

static ALWAYS_INLINE struct k_futex *posix_cond_futex(pthread_cond_t *cv)
{
	return (struct k_futex *)&cv->seq;
}

/* k_futex has no priority inheritance, so PTHREAD_PRIO_INHERIT mutexes stay on k_mutex */
static ALWAYS_INLINE bool posix_mutex_is_pi(const pthread_mutex_t *m)
{
#ifdef PTHREAD_PRIO_INHERIT
	return m->protocol == PTHREAD_PRIO_INHERIT;
#else
	ARG_UNUSED(m);
	return false;
#endif
}

static ALWAYS_INLINE int posix_futex_mutex_lock(pthread_mutex_t *m, k_timeout_t timeout)
{
	int ret;
	atomic_val_t val;
	pthread_t self = 0;

	if (m->type != PTHREAD_MUTEX_NORMAL) {
		self = pthread_self();
		if ((atomic_get(&m->val) != POSIX_MUTEX_UNLOCKED) && (m->owner == self)) {
			if (m->type == PTHREAD_MUTEX_ERRORCHECK) {
				return EDEADLK;
			}
			if (m->count == UINT16_MAX) {
				return EAGAIN;
			}
			m->count++;
			return 0;
		}
	}

	if (!atomic_cas(&m->val, POSIX_MUTEX_UNLOCKED, POSIX_MUTEX_LOCKED)) {
		if (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			return EBUSY;
		}

		val = atomic_set(&m->val, POSIX_MUTEX_CONTENDED);
		while (val != POSIX_MUTEX_UNLOCKED) {
			ret = k_futex_wait(posix_mutex_futex(m), POSIX_MUTEX_CONTENDED, timeout);
			if (ret == -ETIMEDOUT) {
				return ETIMEDOUT;
			}
			if ((ret < 0) && (ret != -EAGAIN)) {
				return EINVAL;
			}
			val = atomic_set(&m->val, POSIX_MUTEX_CONTENDED);
		}
	}

	if (m->type != PTHREAD_MUTEX_NORMAL) {
		m->owner = self;
		m->count = 0;
	}

	return 0;
}

static ALWAYS_INLINE int posix_futex_mutex_unlock(pthread_mutex_t *m)
{
	if (atomic_get(&m->val) == POSIX_MUTEX_UNLOCKED) {
		return EPERM;
	}

	if (m->type != PTHREAD_MUTEX_NORMAL) {
		if (m->owner != pthread_self()) {
			return EPERM;
		}
		if (m->count > 0) {
			m->count--;
			return 0;
		}
		m->owner = 0;
	}

	if (atomic_dec(&m->val) != POSIX_MUTEX_LOCKED) {
		atomic_set(&m->val, POSIX_MUTEX_UNLOCKED);
		(void)k_futex_wake(posix_mutex_futex(m), false);
	}

	return 0;
}

static ALWAYS_INLINE int pthread_mutex_lock_common(pthread_mutex_t *m, k_timeout_t timeout)
{
	if (posix_mutex_is_pi(m)) {
		return -k_mutex_lock(to_k_mutex(m), timeout);
	}

	return posix_futex_mutex_lock(m, timeout);
}

/* k_futex_wait() compares against an int: keep the sequence within its range */
static ALWAYS_INLINE void posix_cond_seq_bump(pthread_cond_t *cv)
{
	if (atomic_inc(&cv->seq) == INT_MAX) {
		(void)atomic_and(&cv->seq, INT_MAX);
	}
}

/*
 * A waiter that has released the mutex but not yet blocked is caught by the
 * sequence compare in k_futex_wait(); one that has blocked is counted by the
 * kernel. Waking a blocked waiter without moving the sequence keeps a signal
 * to one thread; the sequence only moves when no blocked waiter was found.
 */
static ALWAYS_INLINE int posix_futex_cond_signal(pthread_cond_t *cv)
{
	if ((atomic_get(&cv->waiters) != 0) && (k_futex_wake(posix_cond_futex(cv), false) > 0)) {
		return 0;
	}

	posix_cond_seq_bump(cv);
	if (atomic_get(&cv->waiters) != 0) {
		(void)k_futex_wake(posix_cond_futex(cv), false);
	}

	return 0;
}

static ALWAYS_INLINE int posix_futex_cond_broadcast(pthread_cond_t *cv)
{
	posix_cond_seq_bump(cv);
	if (atomic_get(&cv->waiters) != 0) {
		(void)k_futex_wake(posix_cond_futex(cv), true);
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

	seq = atomic_get(&cv->seq);

	ret = pthread_mutex_unlock(m);
	if (ret != 0) {
		return ret;
	}

	ret = k_futex_wait(posix_cond_futex(cv), (int)seq, timeout);
	(void)pthread_mutex_lock(m);

	return (ret == -ETIMEDOUT) ? ETIMEDOUT : 0;
}

static ALWAYS_INLINE int cond_wait(pthread_cond_t *cvar, pthread_mutex_t *mu, clockid_t clock_id,
				   const struct timespec *abstime)
{
	uint32_t clock;

	if (abstime == NULL) {
		return posix_futex_cond_wait(cvar, mu, K_FOREVER);
	}

	if (clock_id == -1) {
		clock = ((cvar->flags & POSIX_COND_CLOCK_MONOTONIC) != 0) ? SYS_CLOCK_MONOTONIC
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

	return posix_futex_cond_wait(cvar, mu, timespec_abs_to_timeout(clock, abstime));
}

#else /* CONFIG_POSIX_THREAD_FUTEX */

static ALWAYS_INLINE int pthread_mutex_lock_common(pthread_mutex_t *m, k_timeout_t timeout)
{
	int ret;

	if (*m == PTHREAD_MUTEX_INITIALIZER) {
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

	if (*mu == PTHREAD_MUTEX_INITIALIZER) {
		ret = pthread_mutex_init(mu, NULL);
		if (ret != 0) {
			return ret;
		}
	}

	if (*cvar == PTHREAD_COND_INITIALIZER) {
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

#endif /* CONFIG_POSIX_THREAD_FUTEX */

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_THREADS_BASE_THREADS_BASE_INTERNAL_H_ */
