/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>

#include "_main.h"

static ZTEST_BMEM pthread_once_t once_control;
static ZTEST_BMEM int once_count;

static void once_func(void)
{
	++once_count;
}

static void *once_fn(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_once(&once_control, once_func));

	return NULL;
}

static void test_pthread_once(void)
{
	pthread_t th;

	once_control = (pthread_once_t)PTHREAD_ONCE_INIT;
	once_count = 0;

	zassert_ok(pthread_once(&once_control, once_func));
	zassert_ok(pthread_once(&once_control, once_func));

	zassert_ok(pthread_create(&th, NULL, once_fn, NULL));
	zassert_ok(pthread_join(th, NULL));

	zassert_equal(once_count, 1, "init routine ran %d times", once_count);
}

ZTEST_THREADS_BASE(test_pthread_once);
