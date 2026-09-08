/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <errno.h>
#include <pthread.h>

#include <zephyr/sys/thread.h>

int pthread_cond_destroy(pthread_cond_t *cvar)
{
#ifdef CONFIG_POSIX_THREAD_FUTEX
	/* woken waiters are already uncounted; a timed-out one uncounts itself before returning */
	while (atomic_get(&cvar->waiters) != 0) {
		k_sleep(K_TICKS(1));
	}

	*cvar = (pthread_cond_t){0};

	return 0;
#else
	return -sys_condvar_destroy(to_k_condvar(cvar));
#endif
}
