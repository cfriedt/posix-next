/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

/**
 * @brief Exactly SYS_THREAD_MUTEX_MIN can be in use at once (when heap allocation is
 * unavailable).
 */
static void mutex_init_resource_exhausted(void)
{
	size_t i;
	pthread_mutex_t m[SYS_THREAD_MUTEX_MIN + 1];

	for (i = 0; i < SYS_THREAD_MUTEX_MIN; ++i) {
		zassert_ok(pthread_mutex_init(&m[i], NULL), "failed to init mutex %zu", i);
	}

	/* try to initialize one more than SYS_THREAD_MUTEX_MIN */
	zassert_equal(i, SYS_THREAD_MUTEX_MIN);

	if (SYS_THREAD_MUTEX_MIN == CONFIG_SYS_THREAD_MUTEX_MAX) {
		/* This test may be removed eventally, since this assertion is successful only when
		 * heap allocation is unavailable, which is non-standard.
		 */
		zassert_not_equal(0, pthread_mutex_init(&m[i], NULL),
				  "should not have initialized mutex %zu", i);
	}

	for (; i > 0; --i) {
		zassert_ok(pthread_mutex_destroy(&m[i - 1]), "failed to destroy mutex %zu", i - 1);
	}
}

static void mutex_init_attr(void)
{
	pthread_mutex_t m;
	pthread_mutexattr_t attr;

	zassert_ok(pthread_mutexattr_init(&attr));
	zassert_ok(pthread_mutex_init(&m, &attr));
	zassert_ok(pthread_mutex_lock(&m));
	zassert_ok(pthread_mutex_unlock(&m));
	zassert_ok(pthread_mutex_destroy(&m));
	zassert_ok(pthread_mutexattr_destroy(&attr));

	IF_NOT_NATIVE_LIBC({
		pthread_mutexattr_t zeroed = {0};

		zassert_equal(pthread_mutex_init(&m, &zeroed), EINVAL);
	})
}

static void test_pthread_mutex_init(void)
{
	mutex_init_attr();

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC) && !k_is_user_context()) {
		mutex_init_resource_exhausted();
	}
}

ZTEST_THREADS_BASE(test_pthread_mutex_init);
