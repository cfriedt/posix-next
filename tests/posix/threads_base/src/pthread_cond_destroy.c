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
 * @brief Demonstrate that condition variables may be used over and over again.
 */
static void test_pthread_cond_destroy(void)
{
	pthread_cond_t cond;

	posix_test_skip_if_native_libc();

	for (size_t i = 0; i < 2 * SYS_THREAD_CONDVAR_MIN; ++i) {
		zassert_ok(pthread_cond_init(&cond, NULL), "failed to init cond %zu", i);
		zassert_ok(pthread_cond_destroy(&cond), "failed to destroy cond %zu", i);
	}
}

ZTEST_THREADS_BASE(test_pthread_cond_destroy);
