/*
 * Copyright (c) 2017 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_clock.h"
#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/thread.h>

static int cond_wait(pthread_cond_t *cvar, pthread_mutex_t *mu, const struct timespec *abstime)
{
	int ret;

	ret = sys_mutex_init((struct k_mutex **)mu, 0);
	if (ret < 0) {
		return -ret;
	}

	ret = sys_condvar_init((struct k_condvar **)cvar);
	if (ret < 0) {
		(void)sys_mutex_destroy((struct k_mutex *)mu);
		return -ret;
	}

	return -k_condvar_wait(*(struct k_condvar **)cvar, *(struct k_mutex **)mu,
			       (abstime == NULL)
				       ? K_FOREVER
				       : sys_timepoint_timeout(timespec_to_timepoint(abstime)));
}

int pthread_cond_signal(pthread_cond_t *cvar)
{
	int ret;

	ret = sys_condvar_init((struct k_condvar **)cvar);
	if (ret < 0) {
		return -ret;
	}

	return -k_condvar_signal(*(struct k_condvar **)cvar);
}

int pthread_cond_broadcast(pthread_cond_t *cvar)
{
	int ret;

	ret = sys_condvar_init((struct k_condvar **)cvar);
	if (ret < 0) {
		return -ret;
	}

	return -k_condvar_broadcast(*(struct k_condvar **)cvar);
}

int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *mut)
{
	return cond_wait(cv, mut, NULL);
}

int pthread_cond_timedwait(pthread_cond_t *cv, pthread_mutex_t *mut, const struct timespec *abstime)
{
	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		return EINVAL;
	}

	return cond_wait(cv, mut, abstime);
}

int pthread_cond_init(pthread_cond_t *cvar, const pthread_condattr_t *att)
{
	struct posix_condattr *attr = (struct posix_condattr *)att;

	if (attr != NULL) {
		if (!attr->initialized) {
			return EINVAL;
		}
	}

	/* FIXME: need to encode the clock into k_condvar */

	return -sys_condvar_init((struct k_condvar **)cvar);
}

int pthread_cond_destroy(pthread_cond_t *cvar)
{
	return -sys_condvar_destroy((struct k_condvar *)cvar);
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
