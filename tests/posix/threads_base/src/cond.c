/*
 * Copyright (c) 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"
#include "_main.h"

/**
 * @brief Test to demonstrate limited condition variable resources
 *
 * @details Exactly SYS_THREAD_CONDVAR_MIN can be in use at once (when heap allocation is
 * unavailable).
 */
ZTEST(posix_threads_base, test_cond_resource_exhausted)
{
	posix_test_skip_if_native_libc();
	size_t i;
	pthread_cond_t m[SYS_THREAD_CONDVAR_MIN + 1];

	for (i = 0; i < SYS_THREAD_CONDVAR_MIN; ++i) {
		zassert_ok(pthread_cond_init(&m[i], NULL), "failed to init cond %zu", i);
	}

	/* try to initialize one more than SYS_THREAD_CONDVAR_MIN */
	zassert_equal(i, SYS_THREAD_CONDVAR_MIN);

	if (SYS_THREAD_CONDVAR_MIN == CONFIG_SYS_THREAD_CONDVAR_MAX) {
		/* This test may be removed eventally, since this assertion is successful only when
		 * heap allocation is unavailable, which is non-standard.
		 */
		zassert_not_equal(0, pthread_cond_init(&m[i], NULL),
				  "should not have initialized cond %zu", i);
	}

	for (; i > 0; --i) {
		zassert_ok(pthread_cond_destroy(&m[i - 1]), "failed to destroy cond %zu", i - 1);
	}
}

/**
 * @brief Test to that there are no condition variable resource leaks
 *
 * @details Demonstrate that condition variables may be used over and over again.
 */
ZTEST(posix_threads_base, test_cond_resource_leak)
{
	posix_test_skip_if_native_libc();
	pthread_cond_t cond;

	for (size_t i = 0; i < 2 * SYS_THREAD_CONDVAR_MIN; ++i) {
		zassert_ok(pthread_cond_init(&cond, NULL), "failed to init cond %zu", i);
		zassert_ok(pthread_cond_destroy(&cond), "failed to destroy cond %zu", i);
	}
}

static void test_pthread_condattr(void)
{
	pthread_condattr_t att = {0};

	zassert_ok(pthread_condattr_init(&att));

	zassert_ok(pthread_condattr_destroy(&att));
}

ZTEST_THREADS_BASE(test_pthread_condattr);

static void test_cond_init_existing_initialized_condattr(void)
{
	pthread_cond_t cond;
	pthread_condattr_t att = {0};

	zassert_ok(pthread_condattr_init(&att));
	zassert_ok(pthread_cond_init(&cond, &att), "pthread_cond_init failed with valid attr");

	/* Clean up */
	zassert_ok(pthread_cond_destroy(&cond));
	zassert_ok(pthread_condattr_destroy(&att));
}

ZTEST_THREADS_BASE(test_cond_init_existing_initialized_condattr);

static void test_cond_broadcast_static_init_pthread_cond_t(void)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	zassert_ok(pthread_cond_broadcast(&cond));
	zassert_ok(pthread_cond_destroy(&cond));
}

ZTEST_THREADS_BASE(test_cond_broadcast_static_init_pthread_cond_t);

static void test_cond_signal_static_init_pthread_cond_t(void)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	zassert_ok(pthread_cond_signal(&cond));
	zassert_ok(pthread_cond_destroy(&cond));
}

ZTEST_THREADS_BASE(test_cond_signal_static_init_pthread_cond_t);
