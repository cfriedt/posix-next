/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_key_delete(pthread_key_t key)
{
	return -k_thread_key_delete((void *)(uintptr_t)key);
}
