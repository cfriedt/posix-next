/*
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <time.h>

#include <zephyr/ztest.h>

ZTEST(xsi_advanced_realtime_threads, test_pthread_getcpuclockid)
{
#if defined(_POSIX_THREAD_CPUTIME)
	clockid_t clock_id = (clockid_t)-1;

	zassert_equal(pthread_getcpuclockid(pthread_self(), NULL), EINVAL);
	zassert_ok(pthread_getcpuclockid(pthread_self(), &clock_id));
	zassert_equal(clock_id, CLOCK_THREAD_CPUTIME_ID);
#else
	ztest_test_skip();
#endif
}

ZTEST_SUITE(xsi_advanced_realtime_threads, NULL, NULL, NULL, NULL, NULL);
