/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void test_pthread_mutexattr_init(void)
{
	pthread_mutexattr_t attr;

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({ zassert_equal(EINVAL, pthread_mutexattr_init(NULL)); })

	zassert_ok(pthread_mutexattr_init(&attr));
	zassert_ok(pthread_mutexattr_destroy(&attr));
}

ZTEST_THREADS_BASE(test_pthread_mutexattr_init);
