/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void test_pthread_mutexattr_getpshared(void)
{
#ifdef _POSIX_THREAD_PROCESS_SHARED
	int pshared = -1;
	pthread_mutexattr_t attr;

	zassert_ok(pthread_mutexattr_init(&attr));

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		zassert_equal(pthread_mutexattr_getpshared(NULL, &pshared), EINVAL);
		zassert_equal(pthread_mutexattr_getpshared(&attr, NULL), EINVAL);
	})

	/* private by default */
	zassert_ok(pthread_mutexattr_getpshared(&attr, &pshared));
	zassert_equal(pshared, PTHREAD_PROCESS_PRIVATE);

	zassert_ok(pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED));
	zassert_ok(pthread_mutexattr_getpshared(&attr, &pshared));
	zassert_equal(pshared, PTHREAD_PROCESS_SHARED);

	zassert_ok(pthread_mutexattr_destroy(&attr));
#else
	ztest_test_skip();
#endif
}

ZTEST_THREADS_BASE(test_pthread_mutexattr_getpshared);
