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

int pthread_attr_getguardsize(const pthread_attr_t *ZRESTRICT _attr, size_t *ZRESTRICT guardsize)
{
	struct posix_thread_attr *const attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || guardsize == NULL) {
		return EINVAL;
	}

	*guardsize = attr->guardsize;

	return 0;
}

int pthread_attr_setguardsize(pthread_attr_t *_attr, size_t guardsize)
{
	struct posix_thread_attr *const attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr)) {
		return EINVAL;
	}

	attr->guardsize = guardsize;

	return 0;
}
