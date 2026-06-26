/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
	const struct pthread_mutexattr *const a = (const struct pthread_mutexattr *)attr;

	if ((a == NULL) || (protocol == NULL) || !a->initialized) {
		return EINVAL;
	}

	*protocol = a->protocol;
	return 0;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if ((a == NULL) || !a->initialized) {
		return EINVAL;
	}

	switch (protocol) {
	case PTHREAD_PRIO_NONE:
		break;
	case PTHREAD_PRIO_INHERIT:
		/* k_mutex implements priority inheritance unconditionally */
		if (!IS_ENABLED(CONFIG_POSIX_THREAD_PRIO_INHERIT)) {
			return ENOTSUP;
		}
		break;
	case PTHREAD_PRIO_PROTECT:
		/* priority ceilings are not implemented */
		return ENOTSUP;
	default:
		return EINVAL;
	}

	a->protocol = protocol;

	return 0;
}
#endif

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
