/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_setspecific(pthread_key_t key, const void *value)
{
	return -k_thread_setspecific((void *)(uintptr_t)key, (void *)value);
}
