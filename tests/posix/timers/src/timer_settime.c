/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timers_tests.h"

static void timer_settime_abstime_monotonic(void)
{
	struct timespec now;
	struct itimerspec its = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));

	/* future absolute deadline */
	zassert_ok(clock_gettime(CLOCK_MONOTONIC, &now));
	its.it_value = now;
	its.it_value.tv_nsec += 2 * PERIOD_NS;
	while (its.it_value.tv_nsec >= (long)NSEC_PER_SEC) {
		its.it_value.tv_nsec -= (long)NSEC_PER_SEC;
		its.it_value.tv_sec++;
	}
	zassert_ok(timer_settime(timerid, TIMER_ABSTIME, &its, NULL));
	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 10 * PERIOD_MS), TEST_SIGNAL_VAL,
		      "future absolute deadline did not fire");

	/*
	 * An absolute deadline in the past expires immediately. The accept
	 * window is an upper bound, not a delay: it matches the file's other
	 * probes so that slow, loaded runners (e.g. -O0 sanitizer binaries
	 * under linux_compat real-time slowdown) do not flake on delivery
	 * latency.
	 */
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 1;
	zassert_ok(timer_settime(timerid, TIMER_ABSTIME, &its, NULL));
	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 10 * PERIOD_MS), TEST_SIGNAL_VAL,
		      "past absolute deadline did not fire immediately");
}

static void timer_settime_abstime_realtime(void)
{
	struct timespec now;
	struct timespec jump;
	struct itimerspec its = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	zassert_ok(timer_create(CLOCK_REALTIME, &sig, &timerid));

	/* absolute wall-clock deadline one hour out */
	zassert_ok(clock_gettime(CLOCK_REALTIME, &now));
	its.it_value = now;
	its.it_value.tv_sec += 3600;
	zassert_ok(timer_settime(timerid, TIMER_ABSTIME, &its, NULL));

	/* jumping the wall clock past the deadline must fire the timer promptly */
	jump = its.it_value;
	jump.tv_sec += 1;
	zassert_ok(clock_settime(CLOCK_REALTIME, &jump));

	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 10 * PERIOD_MS), TEST_SIGNAL_VAL,
		      "clock_settime() past the deadline did not fire the timer");

	/* restore the wall clock roughly */
	zassert_ok(clock_settime(CLOCK_REALTIME, &now));
}

static void timer_settime_abstime_large_realtime(void)
{
	struct timespec orig;
	struct itimerspec its = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};
	const struct timespec ref = {.tv_sec = 1700000000};

	zassert_ok(timer_create(CLOCK_REALTIME, &sig, &timerid));

	/*
	 * A wall clock of 1.7e9 s scaled to nanoseconds overflows 32-bit time_t arithmetic,
	 * and a multi-second offset keeps the overflow from cancelling in the subtraction of
	 * deadline and now: an unwidened conversion fires at once or truncates the wait.
	 */
	zassert_ok(clock_gettime(CLOCK_REALTIME, &orig));
	zassert_ok(clock_settime(CLOCK_REALTIME, &ref));
	its.it_value.tv_sec = ref.tv_sec + 5;
	zassert_ok(timer_settime(timerid, TIMER_ABSTIME, &its, NULL));

	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 2000), -1,
		      "5 s absolute deadline fired within 2 s");
	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 3500 + 10 * PERIOD_MS), TEST_SIGNAL_VAL,
		      "5 s absolute deadline did not fire");

	zassert_ok(clock_settime(CLOCK_REALTIME, &orig));
}

static void timer_settime_errors(void)
{
	struct itimerspec its = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_NONE,
	};

	if (k_is_user_context()) {
		/* a handle that is not a live timer: kernel-mode callers are trusted, the host faults */
		zassert_equal(timer_settime(INVALID_TIMERID, 0, &its, NULL), -1);
		zassert_equal(errno, EINVAL);
	}

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));

	its.it_value.tv_nsec = NSEC_PER_SEC; /* invalid */
	zassert_equal(timer_settime(timerid, 0, &its, NULL), -1);
	zassert_equal(errno, EINVAL);

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* NULL-argument handling is unspecified; the host may fault instead */
		zassert_equal(timer_settime(timerid, 0, NULL, NULL), -1);
		zassert_equal(errno, EINVAL);
	}
}

ZTEST_USER(posix_timers, test_timer_settime)
{
	timer_settime_errors();
	section_reset();

	if (!IS_ENABLED(CONFIG_TIMER_SIGNAL)) {
		/* expiry observation below is signal-based */
		return;
	}

	timer_settime_abstime_monotonic();
	section_reset();

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the host wall clock cannot be jumped from a test */
		timer_settime_abstime_realtime();
		section_reset();

		timer_settime_abstime_large_realtime();
		section_reset();
	}
}
