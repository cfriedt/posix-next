/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <stdbool.h>

#include <zephyr/sys/util.h>
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

static ZTEST_BMEM int exit_order[2];
static ZTEST_BMEM int exit_calls;

static void cleanup_record(void *arg)
{
	exit_order[exit_calls++] = POINTER_TO_INT(arg);
}

static void *cleanup_exit_fn(void *arg)
{
	ARG_UNUSED(arg);

	pthread_cleanup_push(cleanup_record, INT_TO_POINTER(1));
	pthread_cleanup_push(cleanup_record, INT_TO_POINTER(2));
	pthread_exit(NULL);
	pthread_cleanup_pop(false);
	pthread_cleanup_pop(false);

	return NULL;
}

/* handlers still pushed when the thread exits run, most recently pushed first */
static void cleanup_push_run_on_exit(void)
{
	pthread_t th;

	exit_calls = 0;
	zassert_ok(pthread_create(&th, NULL, cleanup_exit_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
	zassert_equal(exit_calls, 2);
	zassert_equal(exit_order[0], 2);
	zassert_equal(exit_order[1], 1);
}

static void test_pthread_cleanup_push(void)
{
	pthread_t th;

	zassert_ok(pthread_create(&th, NULL, cleanup_push_fn, NULL));
	zassert_ok(pthread_join(th, NULL));

	cleanup_push_run_on_exit();
}

ZTEST_THREADS_BASE(test_pthread_cleanup_push);
