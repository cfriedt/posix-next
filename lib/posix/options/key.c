/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	int ret;

	ret = k_thread_key_create((void **)key, destructor);

	/* out of pool slots means PTHREAD_KEYS_MAX is exceeded */
	return (ret == -ENOMEM) ? EAGAIN : -ret;
}

int pthread_key_delete(pthread_key_t key)
{
	return -k_thread_key_delete((void *)(uintptr_t)key);
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
	return -k_thread_setspecific((void *)(uintptr_t)key, (void *)value);
}

void *pthread_getspecific(pthread_key_t key)
{
	void *value = NULL;

	if (k_thread_getspecific((void *)(uintptr_t)key, &value) < 0) {
		return NULL;
	}

	return value;
}
