/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timers_tests.h"

#include <zephyr/sys/timeutil.h>

#define CLOCK_INVALID -1

/* Set a particular time.  In this case, the output of: `date +%s -d 2018-01-01T15:45:01Z` */
static const struct timespec ref_ts = {1514821501, NSEC_PER_SEC / 2U};

static const clockid_t clocks[] = {
#if defined(_POSIX_MONOTONIC_CLOCK)
	CLOCK_MONOTONIC,
#endif
	CLOCK_REALTIME,
};

static const bool settable[] = {
#if defined(_POSIX_MONOTONIC_CLOCK)
	false,
#endif
	true,
};

ZTEST(posix_timers, test_clock_settime)
{
	struct timespec diff;
	struct timespec ts = {0};

	BUILD_ASSERT(ARRAY_SIZE(settable) == ARRAY_SIZE(clocks));

	/* setting CLOCK_REALTIME would modify the host clock (and requires privileges) */
	posix_test_skip_if_native_libc();

	/* ensure argument validation is performed */
	errno = 0;
	zassert_equal(clock_settime(CLOCK_INVALID, &ts), -1);
	zassert_equal(errno, EINVAL);

	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			errno = 0;
			zassert_equal(clock_settime(CLOCK_REALTIME, NULL), -1);
			zassert_equal(errno, EINVAL);
		}
	})

	/* verify nanoseconds */
	errno = 0;
	ts = (struct timespec){0, NSEC_PER_SEC};
	zassert_equal(clock_settime(CLOCK_REALTIME, &ts), -1);
	zassert_equal(errno, EINVAL);
	errno = 0;
	ts = (struct timespec){0, -1};
	zassert_equal(clock_settime(CLOCK_REALTIME, &ts), -1);
	zassert_equal(errno, EINVAL);

	ARRAY_FOR_EACH(clocks, i) {
		if (!settable[i]) {
			/* should fail attempting to set unsettable clocks */
			errno = 0;
			zassert_equal(clock_settime(clocks[i], &ts), -1);
			zassert_equal(errno, EINVAL);
			continue;
		}

		zassert_ok(clock_settime(clocks[i], &ref_ts));

		/* read-back the time */
		zassert_ok(clock_gettime(clocks[i], &ts));
		/* dt should be >= 0, but definitely <= 1s */
		diff = ts;
		zassert_true(timespec_sub(&diff, &ref_ts));
		zassert_true(timespec_compare(&diff, &SYS_TIMESPEC(0, 0)) >= 0);
		zassert_true(timespec_compare(&diff, &SYS_TIMESPEC(1, 0)) <= 0);
	}
}
