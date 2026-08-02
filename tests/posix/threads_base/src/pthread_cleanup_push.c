/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <stdbool.h>

#include <zephyr/ztest.h>

#include "_main.h"

static void cleanup_handler(void *arg)
{
	bool *boolp = (bool *)arg;

	*boolp = true;
}

static void *cleanup_push_fn(void *arg)
{
	bool executed[2] = {0};

	ARG_UNUSED(arg);

	pthread_cleanup_push(cleanup_handler, &executed[0]);
	pthread_cleanup_push(cleanup_handler, &executed[1]);
	pthread_cleanup_pop(false);
	pthread_cleanup_pop(true);

	zassert_true(executed[0]);
	zassert_false(executed[1]);

	return NULL;
}

static void test_pthread_cleanup_push(void)
{
	pthread_t th;

	zassert_ok(pthread_create(&th, NULL, cleanup_push_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
}

ZTEST_THREADS_BASE(test_pthread_cleanup_push);
