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

int pthread_attr_setstack(pthread_attr_t *_attr, void *stackaddr, size_t stacksize)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (stackaddr == NULL) {
		return EINVAL;
	}

	if (!posix_thread_attr_is_valid(attr) || (stacksize < PTHREAD_STACK_MIN)) {
		return EINVAL;
	}

	attr->stack = stackaddr;
	attr->stacksize = stacksize;

	return 0;
}

int pthread_attr_getstack(const pthread_attr_t *_attr, void **stackaddr, size_t *stacksize)
{
	const struct posix_thread_attr *attr = (const struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || (stackaddr == NULL) || (stacksize == NULL)) {
		return EINVAL;
	}

	*stackaddr = attr->stack;
	if (attr->stack == NULL) {
		*stacksize = 0;
	} else {
		*stacksize = attr->stacksize;
	}

	return 0;
}
