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

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_THREADS_BASE_THREADS_BASE_INTERNAL_H_ */
