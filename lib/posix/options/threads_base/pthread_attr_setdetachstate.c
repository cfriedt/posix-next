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

int pthread_attr_setdetachstate(pthread_attr_t *_attr, int detachstate)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || ((detachstate != PTHREAD_CREATE_DETACHED) &&
						  (detachstate != PTHREAD_CREATE_JOINABLE))) {
		return EINVAL;
	}

	attr->detachstate = detachstate;
	return 0;
}
