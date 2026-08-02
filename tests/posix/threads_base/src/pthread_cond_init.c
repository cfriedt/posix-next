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
 * @brief Exactly SYS_THREAD_CONDVAR_MIN can be in use at once (when heap allocation is
 * unavailable).
 */
static void cond_init_resource_exhausted(void)
{
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

static void cond_init_attr(void)
{
	pthread_cond_t cond;
	pthread_condattr_t att = {0};

	IF_NOT_NATIVE_LIBC({
		zassert_equal(pthread_cond_init(&cond, &att), EINVAL);
	})

	zassert_ok(pthread_condattr_init(&att));
	zassert_ok(pthread_cond_init(&cond, &att), "pthread_cond_init failed with valid attr");

	/* Clean up */
	zassert_ok(pthread_cond_destroy(&cond));
	zassert_ok(pthread_condattr_destroy(&att));
}

static void test_pthread_cond_init(void)
{
	cond_init_attr();

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC) && !k_is_user_context()) {
		cond_init_resource_exhausted();
	}
}

ZTEST_THREADS_BASE(test_pthread_cond_init);
