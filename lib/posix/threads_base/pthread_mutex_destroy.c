/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <errno.h>
#include <pthread.h>

#include <zephyr/sys/thread.h>

#ifdef CONFIG_POSIX_THREAD_FUTEX
int pthread_mutex_destroy(pthread_mutex_t *mu)
{
	int ret;

	if (posix_mutex_is_pi(mu)) {
		ret = sys_mutex_destroy(to_k_mutex(mu));
		if (ret < 0) {
			return -ret;
		}
	} else if (atomic_get(&mu->val) != POSIX_MUTEX_UNLOCKED) {
		return EBUSY;
	}

	*mu = (pthread_mutex_t){0};

	return 0;
}
#else
int pthread_mutex_destroy(pthread_mutex_t *mu)
{
	return -sys_mutex_destroy(to_k_mutex(mu));
}
#endif
