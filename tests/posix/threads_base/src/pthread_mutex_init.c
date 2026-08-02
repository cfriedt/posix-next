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
}

ZTEST_THREADS_BASE(test_pthread_mutex_init);
