/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/thread.h>

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	return sys_thread_key_create((sys_thread_key_t *)key, destructor);
}

int pthread_key_delete(pthread_key_t key)
{
	return sys_thread_key_delete((sys_thread_key_t)key);
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
	return sys_thread_setspecific((sys_thread_key_t)key, value);
}

void *pthread_getspecific(pthread_key_t key)
{
	void *value = NULL;

	if (sys_thread_getspecific((sys_thread_key_t)key, &value) < 0) {
		return NULL;
	}

	return value;
}
