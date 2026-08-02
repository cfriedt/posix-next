/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "_main.h"

static void key_delete_no_leak(void)
{
	pthread_key_t leak_key;

	for (size_t i = 0; i < CONFIG_POSIX_THREAD_KEYS_MAX; ++i) {
		zassert_ok(pthread_key_create(&leak_key, NULL), "failed to create key %zu", i);
		zassert_ok(pthread_key_delete(leak_key), "failed to delete key %zu", i);
	}
}

static void key_delete_correct_key(void)
{
	pthread_key_t deleted_key;
	size_t j = CONFIG_POSIX_THREAD_KEYS_MAX - 1;
	pthread_key_t all_keys[CONFIG_POSIX_THREAD_KEYS_MAX];

	for (size_t i = 0; i < ARRAY_SIZE(all_keys); ++i) {
		zassert_ok(pthread_key_create(&all_keys[i], NULL), "failed to create key %zu", i);
	}

	deleted_key = all_keys[j];
	zassert_ok(pthread_key_delete(all_keys[j]));
	zassert_ok(pthread_key_create(&all_keys[j], NULL), "failed to create key %zu", j);

	zassert_equal(deleted_key, all_keys[j], "deleted key %lx instead of key %lx",
		      (long)all_keys[j], (long)deleted_key);

	for (size_t i = 0; i < ARRAY_SIZE(all_keys); ++i) {
		zassert_ok(pthread_key_delete(all_keys[i]), "failed to delete key %zu", i);
	}
}

static void test_pthread_key_delete(void)
{
	if (!k_is_user_context()) {
		key_delete_no_leak();
	}

	key_delete_correct_key();
}

ZTEST_THREADS_BASE(test_pthread_key_delete);
