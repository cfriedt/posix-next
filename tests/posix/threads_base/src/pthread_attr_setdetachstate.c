/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

#define INVALID_DETACHSTATE 7373

static void test_pthread_attr_setdetachstate(void)
{
	int detachstate = PTHREAD_CREATE_JOINABLE;

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setdetachstate(NULL, INVALID_DETACHSTATE),
				      EINVAL);
			zassert_equal(pthread_attr_setdetachstate(NULL, detachstate), EINVAL);
			zassert_equal(pthread_attr_setdetachstate((pthread_attr_t *)&uninit_attr,
								  detachstate),
				      EINVAL);
		}
		/* glibc does not return EINVAL setting an invalid detachstate (POSIX
		 * non-conformance)
		 */
		zassert_equal(pthread_attr_setdetachstate(&test_attr, INVALID_DETACHSTATE), EINVAL);
	})

	/* read back detachstate just written */
	zassert_ok(pthread_attr_setdetachstate(&test_attr, PTHREAD_CREATE_DETACHED));
	zassert_ok(pthread_attr_getdetachstate(&test_attr, &detachstate));
	zassert_equal(detachstate, PTHREAD_CREATE_DETACHED);
	create_thread_common(&test_attr, true, false);
}

ZTEST_THREADS_BASE(test_pthread_attr_setdetachstate);
