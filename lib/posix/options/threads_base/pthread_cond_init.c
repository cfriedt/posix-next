/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <zephyr/sys/clock.h>
#include <zephyr/sys/thread.h>

int pthread_cond_init(pthread_cond_t *cvar, const pthread_condattr_t *att)
{
	int ret;
	struct k_condvar *cond;
	uint32_t sys_clock_id = SYS_CLOCK_REALTIME;
	struct posix_condattr *const attr = (struct posix_condattr *)att;

	if (attr != NULL) {
		if (!attr->initialized) {
			return EINVAL;
		}

		switch (attr->clock) {
		case CLOCK_REALTIME:
			sys_clock_id = SYS_CLOCK_REALTIME;
			break;
		case CLOCK_MONOTONIC:
			sys_clock_id = SYS_CLOCK_MONOTONIC;
			break;
		default:
			return EINVAL;
		}
	}

	ret = sys_condvar_alloc(&cond, sys_clock_id);
	if (ret < 0) {
		return -ret;
	}

	*cvar = (pthread_cond_t)(uintptr_t)cond;

	return 0;
}
