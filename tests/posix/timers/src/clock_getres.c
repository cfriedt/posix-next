/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timers_tests.h"

#include <zephyr/sys/timeutil.h>

#define CLOCK_INVALID -1

ZTEST(posix_timers, test_clock_getres)
{
	int ret;
	struct timespec res;
	const struct timespec one_ns = {
		.tv_sec = 0,
		.tv_nsec = 1,
	};

	struct arg {
		clockid_t clock_id;
		struct timespec *res;
		int expect;
	};

	const struct arg args[] = {
		/* permuting over "invalid" inputs */
		{CLOCK_INVALID, NULL, -1},
		{CLOCK_INVALID, &res, -1},
		{CLOCK_REALTIME, NULL, 0},
		{CLOCK_MONOTONIC, NULL, 0},
		{CLOCK_PROCESS_CPUTIME_ID, NULL, 0},

		/* all valid inputs */
		{CLOCK_REALTIME, &res, 0},
		{CLOCK_MONOTONIC, &res, 0},
		{CLOCK_PROCESS_CPUTIME_ID, &res, 0},
	};

	ARRAY_FOR_EACH_PTR(args, arg) {
		errno = 0;
		res = (struct timespec){0};
		ret = clock_getres(arg->clock_id, arg->res);
		zassert_equal(ret, arg->expect);
		if (ret != 0) {
			zassert_equal(errno, EINVAL);
			continue;
		}
		if (arg->res != NULL) {
			zassert_true(timespec_compare(arg->res, &one_ns) >= 0);
		}
	}
}
