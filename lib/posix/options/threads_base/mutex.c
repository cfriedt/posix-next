/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"
#include "posix_clock.h"

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/mutex.h>
#include <zephyr/sys/timeutil.h>

/* PTHREAD_MUTEX_INITIALIZER spells out the K_MUTEX_NORMAL options of a struct sys_mutex */
BUILD_ASSERT(K_MUTEX_NORMAL == 1);

int pthread_mutex_destroy(pthread_mutex_t *mu)
{
	return -sys_mutex_destroy(mu);
}

static int pthread_mutexattr_to_options(const pthread_mutexattr_t *attr, uint32_t *options)
{
	const struct pthread_mutexattr *const a = (const struct pthread_mutexattr *)attr;

	*options = K_MUTEX_NORMAL;
	if (a == NULL) {
		return 0;
	}

	if (!a->initialized) {
		return -EINVAL;
	}

	switch (a->type) {
	case PTHREAD_MUTEX_DEFAULT:
	case PTHREAD_MUTEX_NORMAL:
		break;
	case PTHREAD_MUTEX_RECURSIVE:
		*options = K_MUTEX_RECURSIVE;
		break;
	case PTHREAD_MUTEX_ERRORCHECK:
		*options = K_MUTEX_ERRORCHECK;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int pthread_mutex_init(pthread_mutex_t *mu, const pthread_mutexattr_t *attr)
{
	int ret;
	uint32_t options;

	if (pthread_mutexattr_to_options(attr, &options) < 0) {
		return EINVAL;
	}

	ret = sys_mutex_init_ext(mu, options);
	if (ret < 0) {
		return -ret;
	}

	return 0;
}

static int pthread_mutex_lock_common(pthread_mutex_t *m, k_timeout_t timeout)
{
	return -sys_mutex_lock(m, timeout);
}

int pthread_mutex_lock(pthread_mutex_t *m)
{
	return pthread_mutex_lock_common(m, K_FOREVER);
}

int pthread_mutex_timedlock(pthread_mutex_t *m,
			    const struct timespec *abstime)
{
	int ret;

	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		return EINVAL;
	}

	ret = pthread_mutex_lock_common(m, sys_timepoint_timeout(timespec_abs_rt_to_timepoint(abstime)));

	if ((ret == EAGAIN) || (ret == EBUSY)) {
		ret = ETIMEDOUT;
	}

	return ret;
}

int pthread_mutex_trylock(pthread_mutex_t *m)
{
	int ret;

	ret = pthread_mutex_lock_common(m, K_NO_WAIT);

	return ((ret == EAGAIN) || (ret == EDEADLK)) ? EBUSY : ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mu)
{
	return -sys_mutex_unlock(mu);
}
#if defined(_POSIX_THREAD_PRIO_PROTECT)

#endif

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if (a == NULL) {
		return EINVAL;
	}

	*a = (struct pthread_mutexattr){
		.type = PTHREAD_MUTEX_DEFAULT,
		.initialized = true,
	};

	return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if (a == NULL || !a->initialized) {
		return EINVAL;
	}

	*a = (struct pthread_mutexattr){0};

	return 0;
}

#if defined(_POSIX_THREAD_PRIO_PROTECT)

#endif
