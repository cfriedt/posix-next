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

#ifdef CONFIG_POSIX_THREAD_FUTEX
int pthread_mutex_init(pthread_mutex_t *mu, const pthread_mutexattr_t *attr)
{
	int ret;
	int flags = 0;
	struct k_mutex *mutex;
	const struct pthread_mutexattr *const a = (const struct pthread_mutexattr *)attr;

	if (pthread_mutexattr_to_flags(attr, &flags) < 0) {
		return EINVAL;
	}

	*mu = (pthread_mutex_t){0};

	if (a != NULL) {
		mu->type = (a->type == PTHREAD_MUTEX_DEFAULT) ? PTHREAD_MUTEX_NORMAL : a->type;
		mu->protocol = a->protocol;
	}

	if (posix_mutex_is_pi(mu)) {
		ret = sys_mutex_alloc(&mutex, flags);
		if (ret < 0) {
			*mu = (pthread_mutex_t){0};
			return -ret;
		}

		mu->val = (atomic_val_t)(uintptr_t)mutex;
	}

	return 0;
}
#else
int pthread_mutex_init(pthread_mutex_t *mu, const pthread_mutexattr_t *attr)
{
	int ret;
	int flags = 0;
	struct k_mutex *mutex;

	if (pthread_mutexattr_to_flags(attr, &flags) < 0) {
		return EINVAL;
	}

	ret = sys_mutex_alloc(&mutex, flags);
	if (ret < 0) {
		return -ret;
	}

	*mu = (pthread_mutex_t)(uintptr_t)mutex;

	return 0;
}
#endif /* CONFIG_POSIX_THREAD_FUTEX */
