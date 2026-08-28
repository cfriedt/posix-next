/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/sys/atomic.h>

static atomic_t pthread_concurrency;

int pthread_getconcurrency(void)
{
	return (int)atomic_get(&pthread_concurrency);
}

int pthread_setconcurrency(int new_level)
{
	if (new_level < 0) {
		return EINVAL;
	}

	if (new_level > CONFIG_MP_MAX_NUM_CPUS) {
		return EAGAIN;
	}

	(void)atomic_set(&pthread_concurrency, new_level);

	return 0;
}
