/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timers_tests.h"

static void timer_gettime_armed_and_expired(void)
{
	struct itimerspec its = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_NONE,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	arm_ms(timerid, 0, 2 * PERIOD_MS, 0);

	zassert_ok(timer_gettime(timerid, &its));
	zassert_true((its.it_value.tv_sec > 0) || (its.it_value.tv_nsec > 0),
		     "armed timer reports disarmed");

	test_sleep_ms(3 * PERIOD_MS);

	/* an expired one-shot reports a zero it_value */
	zassert_ok(timer_gettime(timerid, &its));
	zassert_true((its.it_value.tv_sec == 0) && (its.it_value.tv_nsec == 0),
		     "expired one-shot still reports armed");
	zassert_equal(timer_getoverrun(timerid), 0);
}

static void timer_gettime_errors(void)
{
	struct sigevent sig = {
		.sigev_notify = SIGEV_NONE,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));

	zassert_equal(timer_gettime(timerid, NULL), -1);
	zassert_equal(errno, EINVAL);
}

ZTEST_USER(posix_timers, test_timer_gettime)
{
	timer_gettime_armed_and_expired();
	section_reset();

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* NULL-argument handling is unspecified; the host may fault instead */
		timer_gettime_errors();
		section_reset();
	}
}
