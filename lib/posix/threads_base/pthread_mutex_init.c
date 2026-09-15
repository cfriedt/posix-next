/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/thread.h>

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

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
	switch (a->protocol) {
	case PTHREAD_PRIO_NONE:
		break;
	case PTHREAD_PRIO_INHERIT:
		*options |= K_MUTEX_PRIO_INHERIT;
		break;
	default:
		return -EINVAL;
	}
#else
	if (a->protocol != 0) {
		return -EINVAL;
	}
#endif

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

	if ((options & K_MUTEX_PRIO_INHERIT) != 0) {
		/* claim the backing kernel mutex now, so that exhaustion is reported here */
		ret = sys_mutex_lock(mu, K_NO_WAIT);
		if (ret < 0) {
			(void)sys_mutex_destroy(mu);
			return (ret == -ENOMEM) ? ENOMEM : EAGAIN;
		}
		(void)sys_mutex_unlock(mu);
	}

	return 0;
}
