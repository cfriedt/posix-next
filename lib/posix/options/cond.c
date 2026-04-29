/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_clock.h"
#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/condvar.h>

static int cond_wait(pthread_cond_t *cvar, pthread_mutex_t *mu, clockid_t clock_id, const struct timespec *abstime)
{
	uint32_t clock;

	if (abstime == NULL) {
		return -sys_condvar_wait(cvar, mu, K_FOREVER);
	}

	if (clock_id == -1) {
		clock = sys_condvar_clock(cvar);
	} else if (clock_id == CLOCK_REALTIME) {
		clock = SYS_CLOCK_REALTIME;
	} else if (clock_id == CLOCK_MONOTONIC) {
		clock = SYS_CLOCK_MONOTONIC;
	} else {
		return EINVAL;
	}

	return -sys_condvar_clockwait(cvar, mu, clock, abstime);
}

int pthread_cond_signal(pthread_cond_t *cvar)
{
	return -sys_condvar_signal(cvar);
}

int pthread_cond_broadcast(pthread_cond_t *cvar)
{
	return -sys_condvar_broadcast(cvar);
}
int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *mut)
{
	return cond_wait(cv, mut, -1, NULL);
}

int pthread_cond_timedwait(pthread_cond_t *cv, pthread_mutex_t *mut, const struct timespec *abstime)
{
	if (abstime == NULL) {
		return EINVAL;
	}

	return cond_wait(cv, mut, -1, abstime);
}

int pthread_cond_clockwait(pthread_cond_t *cv, pthread_mutex_t *mut, clockid_t clock_id, const struct timespec *abstime)
{
	if (abstime == NULL) {
		return EINVAL;
	}

	return cond_wait(cv, mut, clock_id, abstime);
}

int pthread_cond_init(pthread_cond_t *cvar, const pthread_condattr_t *att)
{
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

	return -sys_condvar_init_ext(cvar, sys_clock_id);
}

int pthread_cond_destroy(pthread_cond_t *cvar)
{
	return -sys_condvar_destroy(cvar);
}

int pthread_condattr_init(pthread_condattr_t *att)
{
	struct posix_condattr *const attr = (struct posix_condattr *)att;

	if ((att == NULL) || attr->initialized) {
		return EINVAL;
	}

	attr->clock = CLOCK_REALTIME;
	attr->initialized = true;

	return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *att)
{
	struct posix_condattr *const attr = (struct posix_condattr *)att;

	if ((attr == NULL) || !attr->initialized) {
		return EINVAL;
	}

	*attr = (struct posix_condattr){0};

	return 0;
}
