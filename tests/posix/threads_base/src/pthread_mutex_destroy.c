/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

/**
 * @brief Demonstrate that mutexes may be used over and over again.
 */
static void test_pthread_mutex_destroy(void)
{
	pthread_mutex_t m;

	posix_test_skip_if_native_libc();

	for (size_t i = 0; i < 2 * SYS_THREAD_MUTEX_MIN; ++i) {
		zassert_ok(pthread_mutex_init(&m, NULL), "failed to init mutex %zu", i);
		zassert_ok(pthread_mutex_destroy(&m), "failed to destroy mutex %zu", i);
	}
}

ZTEST_THREADS_BASE(test_pthread_mutex_destroy);
