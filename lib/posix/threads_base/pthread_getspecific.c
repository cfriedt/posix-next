/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>

void *pthread_getspecific(pthread_key_t key)
{
	void *value = NULL;

	if (k_thread_getspecific((void *)(uintptr_t)key, &value) < 0) {
		return NULL;
	}

	return value;
}
