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

static int pthread_mutexattr_to_flags(const pthread_mutexattr_t *attr, int *flags)
{
	*flags = 0;

	if (attr == NULL) {
		*flags |= K_MUTEX_NORMAL;
		return 0;
	}

	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if (!a->initialized) {
		return -EINVAL;
	}

	switch (a->type) {
	case PTHREAD_MUTEX_DEFAULT:
	case PTHREAD_MUTEX_NORMAL:
		*flags |= K_MUTEX_NORMAL;
		break;
#if defined(PTHREAD_MUTEX_ROBUST)
	case PTHREAD_MUTEX_ROBUST:
#endif
		break;
	case PTHREAD_MUTEX_RECURSIVE:
		*flags |= K_MUTEX_RECURSIVE;
		break;
	case PTHREAD_MUTEX_ERRORCHECK:
		*flags |= K_MUTEX_ERRORCHECK;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int pthread_mutex_init(pthread_mutex_t *mu, const pthread_mutexattr_t *attr)
{
	int ret;
	int flags = 0;
	struct k_mutex *mutex;

	if (pthread_mutexattr_to_flags(attr, &flags) < 0) {
		return EINVAL;
	}

	if (IS_ENABLED(CONFIG_POSIX_THREAD_FUTEX)) {
		return posix_futex_mutex_init(mu, (const struct pthread_mutexattr *)attr, flags);
	}

	ret = sys_mutex_alloc(&mutex, flags);
	if (ret < 0) {
		return -ret;
	}

	posix_mutex_set_handle(mu, mutex);

	return 0;
}
