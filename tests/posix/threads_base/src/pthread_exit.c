/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "_main.h"

static void *pthread_exit_fn(void *arg)
{
	pthread_exit(arg);

	zassert_unreachable();
	return NULL;
}

static void test_pthread_exit(void)
{
	pthread_t th;
	void *retval = NULL;

	zassert_ok(pthread_create(&th, NULL, pthread_exit_fn, INT_TO_POINTER(42)));
	zassert_ok(pthread_join(th, &retval));
	zassert_equal(POINTER_TO_INT(retval), 42);
}

ZTEST_THREADS_BASE(test_pthread_exit);
