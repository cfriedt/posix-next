/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/thread.h>

int pthread_once(pthread_once_t *once, void (*init_func)(void))
{
	sys_thread_once((sys_thread_once_t *)once, init_func);

	return 0;
}
