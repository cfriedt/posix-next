/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void test_pthread_attr_init(void)
{
	/* test_attr has already been initialized in before() */
	can_create_thread(&test_attr);

	zassert_ok(pthread_attr_destroy(&test_attr));
	test_attr_valid = false;

	zassert_ok(pthread_attr_init(&test_attr));
	test_attr_valid = true;

	can_create_thread(&test_attr);

	/* note: test_attr is still valid and is destroyed in after() */
}

ZTEST_THREADS_BASE(test_pthread_attr_init);
