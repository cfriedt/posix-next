/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <pthread.h>

int pthread_attr_init(pthread_attr_t *attr)
{
	posix_thread_attr_init((struct posix_thread_attr *)attr);
	return 0;
}
