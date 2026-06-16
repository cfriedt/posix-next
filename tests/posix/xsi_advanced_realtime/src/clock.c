/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>

#include <zephyr/ztest.h>

#define CLOCK_INVALID ((clockid_t)-1)

ZTEST(xsi_advanced_realtime, test_clock_getcpuclockid)
{
#if defined(_POSIX_CPUTIME)
	int ret;
	clockid_t clock_id = CLOCK_INVALID;

	ret = clock_getcpuclockid((pid_t)0, &clock_id);
	zassert_equal(ret, 0, "POSIX clock_getcpuclock id failed");
	zassert_equal(clock_id, CLOCK_PROCESS_CPUTIME_ID, "POSIX clock_getcpuclock id failed");

	ret = clock_getcpuclockid((pid_t)2482, &clock_id);
	zassert_equal(ret, EPERM, "POSIX clock_getcpuclock id failed");
#else
	ztest_test_skip();
#endif
}

ZTEST_SUITE(xsi_advanced_realtime, NULL, NULL, NULL, NULL, NULL);
