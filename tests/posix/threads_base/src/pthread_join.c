/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "_main.h"

static void test_pthread_join(void)
{
	void *retval;

	zassert_equal(pthread_join(pthread_self(), &retval), EDEADLK);
}

ZTEST_THREADS_BASE(test_pthread_join);
