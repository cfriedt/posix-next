/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timers_tests.h"

ZTEST_USER(posix_timers, test_timer_getoverrun)
{
	if (!IS_ENABLED(CONFIG_TIMER_SIGNAL)) {
		/* signal-based expiry notification requires the kernel signal subsystem */
		ztest_test_skip();
	}
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the host cannot block process-directed signals across simulator threads */
		ztest_test_skip();
	}
	int overrun;
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	arm_ms(timerid, 0, PERIOD_MS, PERIOD_MS);

	/* let ~5 periods elapse with the (blocked) signal left pending */
	test_sleep_ms(5 * PERIOD_MS + PERIOD_MS / 2);

	/* exactly one instance is queued; accepting it latches the overrun */
	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 0), TEST_SIGNAL_VAL);
	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 0), -1,
		      "more than one signal instance queued for one timer");

	overrun = timer_getoverrun(timerid);
	zassert_true((overrun >= 2) && (overrun <= 6), "overrun %d outside expected window",
		     overrun);
	/* non-destructive: repeated reads return the latched value */
	zassert_equal(timer_getoverrun(timerid), overrun);
	zassert_equal(timer_getoverrun(timerid), overrun);
}
