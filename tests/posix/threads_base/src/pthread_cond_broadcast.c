/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>

#include "_main.h"

static void test_pthread_cond_broadcast(void)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	zassert_ok(pthread_cond_broadcast(&cond));
	zassert_ok(pthread_cond_destroy(&cond));
}

ZTEST_THREADS_BASE(test_pthread_cond_broadcast);
