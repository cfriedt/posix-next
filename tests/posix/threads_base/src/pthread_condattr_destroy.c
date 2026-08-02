/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void test_pthread_condattr_destroy(void)
{
	pthread_condattr_t att = {0};

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({ zassert_equal(pthread_condattr_destroy(NULL), EINVAL); })

	zassert_ok(pthread_condattr_init(&att));

	zassert_ok(pthread_condattr_destroy(&att));
}

ZTEST_THREADS_BASE(test_pthread_condattr_destroy);
