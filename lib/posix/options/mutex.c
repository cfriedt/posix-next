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

struct pthread_mutexattr {
	unsigned char type: 2;
	bool initialized: 1;
};
BUILD_ASSERT(sizeof(pthread_mutexattr_t) >= sizeof(struct pthread_mutexattr));

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
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr,
				  int *protocol)
{
	if ((attr == NULL) || (protocol == NULL)) {
		return EINVAL;
	}

	*protocol = PTHREAD_PRIO_NONE;
	return 0;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
	if (attr == NULL) {
		return EINVAL;
	}

	switch (protocol) {
	case PTHREAD_PRIO_NONE:
		return 0;
	case PTHREAD_PRIO_INHERIT:
		return ENOTSUP;
	case PTHREAD_PRIO_PROTECT:
		return ENOTSUP;
	default:
		return EINVAL;
	}
}
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

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
	const struct pthread_mutexattr *a = (const struct pthread_mutexattr *)attr;

	if (a == NULL || type == NULL || !a->initialized) {
		return EINVAL;
	}

	*type = a->type;

	return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if (a == NULL || !a->initialized) {
		return EINVAL;
	}

	switch (type) {
	case PTHREAD_MUTEX_NORMAL:
	case PTHREAD_MUTEX_RECURSIVE:
	case PTHREAD_MUTEX_ERRORCHECK:
	case PTHREAD_MUTEX_DEFAULT:
		a->type = type;
		return 0;
	default:
		return EINVAL;
	}
}

#if defined(_POSIX_THREAD_PRIO_PROTECT)
int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling)
{
	ARG_UNUSED(mutex);
	ARG_UNUSED(prioceiling);

	return ENOSYS;
}

int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling)
{
	ARG_UNUSED(mutex);
	ARG_UNUSED(prioceiling);
	ARG_UNUSED(old_ceiling);

	return ENOSYS;
}

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling)
{
	ARG_UNUSED(attr);
	ARG_UNUSED(prioceiling);

	return ENOSYS;
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
	ARG_UNUSED(attr);
	ARG_UNUSED(prioceiling);

	return ENOSYS;
}

#endif
