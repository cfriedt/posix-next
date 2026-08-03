/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

ZTEST_USER(xsi_advanced_realtime_threads, test_pthread_getcpuclockid)
{
#if defined(_POSIX_THREAD_CPUTIME)
	clockid_t clock_id = (clockid_t)-1;

	IF_NOT_NATIVE_LIBC({
		zassert_equal(pthread_getcpuclockid(pthread_self(), NULL), EINVAL);
	})

	zassert_ok(pthread_getcpuclockid(pthread_self(), &clock_id));

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* Zephyr maps every thread's CPU-time clock onto the same clock id */
		zassert_equal(clock_id, CLOCK_THREAD_CPUTIME_ID);
	}
#else
	ztest_test_skip();
#endif
}

ZTEST_SUITE(xsi_advanced_realtime_threads, NULL, NULL, NULL, NULL, NULL);
