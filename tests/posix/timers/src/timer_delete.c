/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timers_tests.h"

ZTEST_USER(posix_timers, test_timer_delete)
{
	struct sigevent sig = {
		.sigev_notify = SIGEV_NONE,
	};

	if (IS_ENABLED(CONFIG_NATIVE_LIBC) || IS_ENABLED(CONFIG_USERSPACE)) {
		/* host libc semantics differ; in user mode a stale handle faults instead */
		ztest_test_skip();
	}

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	zassert_ok(timer_delete(timerid));

	/* double delete must fail cleanly */
	zassert_equal(timer_delete(timerid), -1);
	zassert_equal(errno, EINVAL);
	timerid = INVALID_TIMERID;
}
