/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void test_pthread_equal(void)
{
	zassert_true(pthread_equal(pthread_self(), pthread_self()));
	zassert_false(pthread_equal(pthread_self(), (pthread_t)4242));
	IF_NOT_NATIVE_LIBC({
		/* not true when running the testsuite against Linux's pthread implementation */
		zassert_true(pthread_equal(pthread_self(), (pthread_t)(uintptr_t)k_current_get()));
	})
}

ZTEST_THREADS_BASE(test_pthread_equal);
