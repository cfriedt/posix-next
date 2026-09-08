/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_cond_signal(pthread_cond_t *cvar)
{
#ifdef CONFIG_POSIX_THREAD_FUTEX
	return posix_futex_cond_signal(cvar);
#else
	int ret;

	if (*cvar == PTHREAD_COND_INITIALIZER) {
		ret = pthread_cond_init(cvar, NULL);
		if (ret != 0) {
			return ret;
		}
	}

	return -k_condvar_signal(to_k_condvar(cvar));
#endif
}
