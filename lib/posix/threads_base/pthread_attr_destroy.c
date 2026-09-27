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

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pthread, CONFIG_PTHREAD_LOG_LEVEL);

/**
 * @brief Destroy thread attributes object.
 *
 * See IEEE 1003.1
 */
int pthread_attr_destroy(pthread_attr_t *_attr)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr)) {
		return EINVAL;
	}

	*attr = (struct posix_thread_attr){0};
	LOG_DBG("Destroyed attr %p", _attr);

	return 0;
}
