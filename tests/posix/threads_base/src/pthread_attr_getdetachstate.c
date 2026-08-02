/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void test_pthread_attr_getdetachstate(void)
{
	int detachstate;

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_getdetachstate(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getdetachstate(NULL, &detachstate), EINVAL);
			zassert_equal(pthread_attr_getdetachstate(&uninit_attr, &detachstate),
				      EINVAL);
		}
		/* avoid glibc non-null compiler warning promoted to error */
		zassert_equal(pthread_attr_getdetachstate(&test_attr, NULL), EINVAL);
	})

	/* default detachstate is joinable */
	zassert_ok(pthread_attr_getdetachstate(&test_attr, &detachstate));
	zassert_equal(detachstate, PTHREAD_CREATE_JOINABLE);
	can_create_thread(&test_attr);
}

ZTEST_THREADS_BASE(test_pthread_attr_getdetachstate);
