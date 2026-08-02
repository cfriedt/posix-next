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
}

ZTEST_THREADS_BASE(test_pthread_cond_init);
