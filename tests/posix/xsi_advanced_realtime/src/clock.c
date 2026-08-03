/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <time.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

#define CLOCK_INVALID ((clockid_t)-1)

ZTEST_USER(xsi_advanced_realtime, test_clock_getcpuclockid)
{
#if defined(_POSIX_CPUTIME)
	clockid_t clock_id = CLOCK_INVALID;

	/* the calling process' CPU-time clock is always accessible */
	zassert_ok(clock_getcpuclockid((pid_t)0, &clock_id));
	zassert_ok(clock_getres(clock_id, NULL));

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* Zephyr is single-process, so it is always CLOCK_PROCESS_CPUTIME_ID */
		zassert_equal(clock_id, CLOCK_PROCESS_CPUTIME_ID);

		/* other processes do not exist, so their clocks are not accessible */
		zassert_equal(clock_getcpuclockid((pid_t)2482, &clock_id), EPERM);
	}
#else
	ztest_test_skip();
#endif
}

ZTEST_SUITE(xsi_advanced_realtime, NULL, NULL, NULL, NULL, NULL);
