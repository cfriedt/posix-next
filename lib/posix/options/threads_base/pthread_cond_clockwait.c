/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <errno.h>
#include <pthread.h>

int pthread_cond_clockwait(pthread_cond_t *cv, pthread_mutex_t *mut, clockid_t clock_id,
			   const struct timespec *abstime)
{
	if (abstime == NULL) {
		return EINVAL;
	}

	return cond_wait(cv, mut, clock_id, abstime);
}
