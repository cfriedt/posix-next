/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
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
		/* out of pool slots means PTHREAD_KEYS_MAX is exceeded */
		return (ret == -ENOMEM) ? EAGAIN : -ret;
	}

	*key = (pthread_key_t)(uintptr_t)kkey;
	return 0;
}
