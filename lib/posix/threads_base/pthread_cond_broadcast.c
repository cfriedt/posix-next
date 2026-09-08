/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_cond_broadcast(pthread_cond_t *cvar)
{
	int ret;

	if (IS_ENABLED(CONFIG_POSIX_THREAD_FUTEX)) {
		return posix_futex_cond_broadcast(cvar);
	}

	if (posix_cond_is_static_init(cvar)) {
		ret = pthread_cond_init(cvar, NULL);
		if (ret != 0) {
			return ret;
		}
	}

	return -k_condvar_broadcast(to_k_condvar(cvar));
}
