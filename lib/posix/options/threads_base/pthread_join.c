/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_join(pthread_t pthread, void **status)
{
	return -k_thread_rejoin(to_k_thread(&pthread), status, K_FOREVER);
}
