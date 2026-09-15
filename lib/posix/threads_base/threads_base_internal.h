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
#include <stddef.h>

#include <zephyr/sys/condvar.h>
#include <zephyr/sys/mutex.h>

/* PTHREAD_MUTEX_INITIALIZER spells out the K_MUTEX_NORMAL options of a struct sys_mutex */
BUILD_ASSERT(K_MUTEX_NORMAL == 1);

static ALWAYS_INLINE int pthread_mutex_lock_common(pthread_mutex_t *m, k_timeout_t timeout)
{
	int ret = sys_mutex_lock(m, timeout);

	return (ret == -ENOMEM) ? EAGAIN : -ret;
}

static ALWAYS_INLINE int cond_wait(pthread_cond_t *cvar, pthread_mutex_t *mu, clockid_t clock_id,
				   const struct timespec *abstime)
{
	uint32_t clock;

	/* cancellation point, taken while the mutex is still held */
	pthread_testcancel();

	if (abstime == NULL) {
		return -sys_condvar_wait(cvar, mu, K_FOREVER);
	}

	if (clock_id == -1) {
		clock = sys_condvar_clock(cvar);
	} else if (clock_id == CLOCK_REALTIME) {
		clock = SYS_CLOCK_REALTIME;
	} else if (clock_id == CLOCK_MONOTONIC) {
		clock = SYS_CLOCK_MONOTONIC;
	} else {
		return EINVAL;
	}

	return -sys_condvar_clockwait(cvar, mu, clock, abstime);
}

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_THREADS_BASE_THREADS_BASE_INTERNAL_H_ */
