/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/thread.h>
#include <zephyr/sys/timeutil.h>

struct pthread_mutexattr {
	unsigned char type: 2;
	bool initialized: 1;
};
BUILD_ASSERT(sizeof(pthread_mutexattr_t) >= sizeof(struct pthread_mutexattr));

int pthread_mutex_destroy(pthread_mutex_t *mu)
{
	return -sys_mutex_destroy(*(struct k_mutex **)mu);
}

static int pthread_mutexattr_to_flags(const pthread_mutexattr_t *attr, int *flags)
{
	if (attr == NULL) {
		*flags = 0;
		return 0;
	}

	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if (!a->initialized) {
		return -EINVAL;
	}

	switch (a->type) {
	case PTHREAD_MUTEX_DEFAULT:
	case PTHREAD_MUTEX_NORMAL:
#if defined(PTHREAD_MUTEX_ROBUST)
	case PTHREAD_MUTEX_ROBUST:
#endif
		break;
	case PTHREAD_MUTEX_RECURSIVE:
		*flags = SYS_MUTEX_RECURSIVE;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int pthread_mutex_init(pthread_mutex_t *mu, const pthread_mutexattr_t *attr)
{
	int flags = 0;

	if (pthread_mutexattr_to_flags(attr, &flags) < 0) {
		return EINVAL;
	}

	return -sys_mutex_init((struct k_mutex **)mu, flags);
}

int pthread_mutex_lock(pthread_mutex_t *m)
{
	return -k_mutex_lock(*(struct k_mutex **)m, K_FOREVER);
}

int pthread_mutex_timedlock(pthread_mutex_t *m,
			    const struct timespec *abstime)
{
	return -k_mutex_lock(*(struct k_mutex **)m,
			     sys_timepoint_timeout(timespec_to_timepoint(abstime)));
}

int pthread_mutex_trylock(pthread_mutex_t *m)
{
	return -k_mutex_lock(*(struct k_mutex **)m, K_NO_WAIT);
}

int pthread_mutex_unlock(pthread_mutex_t *mu)
{
	return -k_mutex_unlock(*(struct k_mutex **)mu);
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
