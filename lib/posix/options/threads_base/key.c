/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

BUILD_ASSERT(CONFIG_POSIX_THREAD_KEYS_MAX <= CONFIG_THREAD_SPECIFIC_STORAGE_KEYS_MAX,
	     "CONFIG_THREAD_SPECIFIC_STORAGE_KEYS_MAX must be >= CONFIG_POSIX_THREAD_KEYS_MAX");

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	void *kkey;
	int ret;

	ret = k_thread_key_create(&kkey, destructor);
	if (ret != 0) {
		return -ret;
	}

	*key = (pthread_key_t)(uintptr_t)kkey;
	return 0;
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
