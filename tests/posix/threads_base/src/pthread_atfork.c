/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>

#include "_main.h"

static void test_pthread_atfork(void)
{
	/*
	 * fork() does not exist, so registered handlers can never run;
	 * registration itself succeeds (and NULL handlers are permitted)
	 */
	zassert_ok(pthread_atfork(NULL, NULL, NULL));
}

ZTEST_THREADS_BASE(test_pthread_atfork);
