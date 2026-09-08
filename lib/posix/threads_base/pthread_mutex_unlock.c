/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_mutex_unlock(pthread_mutex_t *mu)
{
#ifdef CONFIG_POSIX_THREAD_FUTEX
	if (!posix_mutex_is_pi(mu)) {
		return posix_futex_mutex_unlock(mu);
	}
#endif

	return -k_mutex_unlock(to_k_mutex(mu));
}
