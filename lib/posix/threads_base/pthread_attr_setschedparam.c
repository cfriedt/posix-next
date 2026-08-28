/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>

int pthread_attr_setschedparam(pthread_attr_t *_attr, const struct sched_param *schedparam)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || (schedparam == NULL) ||
	    !is_posix_policy_prio_valid(schedparam->sched_priority, attr->schedpolicy)) {
		return EINVAL;
	}

#ifdef SCHED_SPORADIC
	if ((attr->schedpolicy == SCHED_SPORADIC) && !posix_sporadic_param_is_valid(schedparam)) {
		return EINVAL;
	}
#endif

	attr->priority = schedparam->sched_priority;
	return 0;
}
