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

#include <zephyr/kernel.h>

int pthread_attr_getstacksize(const pthread_attr_t *_attr, size_t *stacksize)
{
	const struct posix_thread_attr *attr = (const struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || (stacksize == NULL)) {
		return EINVAL;
	}

	*stacksize = ((attr->stack == NULL) && (attr->stacksize == 0))
			     ? MAX((size_t)CONFIG_SYS_THREAD_STACK_SIZE, (size_t)PTHREAD_STACK_MIN)
			     : attr->stacksize;

	return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *_attr, size_t stacksize)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || (stacksize < PTHREAD_STACK_MIN)) {
		return EINVAL;
	}

	if (attr->stack != NULL) {
		return EINVAL;
	}

	attr->stacksize = stacksize;

	return 0;
}
