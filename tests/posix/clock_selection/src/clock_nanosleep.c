/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"
#include "../../common/nanosleep_common.h"

static void clock_nanosleep_execution(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	/* absolute sleeps with the monotonic clock and reference time ts */

	/* until 1s + 1ns past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_MONOTONIC, TIMER_ABSTIME,
				 ts.tv_sec + 1, 1);

	/* until 1s + 1us past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_MONOTONIC, TIMER_ABSTIME,
				 ts.tv_sec + 1, 1000);

	/* until 1s + 500000000ns past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_MONOTONIC, TIMER_ABSTIME,
				 ts.tv_sec + 1, 500000000);

	/* until 2s past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_MONOTONIC, TIMER_ABSTIME,
				 ts.tv_sec + 2, 0);

	/* until 2s + 1ns past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_MONOTONIC, TIMER_ABSTIME,
				 ts.tv_sec + 2, 1);

	/* until 2s + 1us + 1ns past reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_MONOTONIC, TIMER_ABSTIME,
				 ts.tv_sec + 2, 1001);

	clock_gettime(CLOCK_REALTIME, &ts);

	/* absolute sleeps with the real time clock and adjusted reference time ts */

	/* until 1s + 1ns past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_REALTIME, TIMER_ABSTIME,
				 ts.tv_sec + 1, 1);

	/* until 1s + 1us past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_REALTIME, TIMER_ABSTIME,
				 ts.tv_sec + 1, 1000);

	/* until 1s + 500000000ns past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_REALTIME, TIMER_ABSTIME,
				 ts.tv_sec + 1, 500000000);

	/* until 2s past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_REALTIME, TIMER_ABSTIME,
				 ts.tv_sec + 2, 0);

	/* until 2s + 1ns past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_REALTIME, TIMER_ABSTIME,
				 ts.tv_sec + 2, 1);

	/* until 2s + 1us + 1ns past the reference time */
	common_lower_bound_check(SELECT_CLOCK_NANOSLEEP, CLOCK_REALTIME, TIMER_ABSTIME,
				 ts.tv_sec + 2, 1001);
}

static void clock_nanosleep_errors_errno(void)
{
	struct timespec rem = {};
	struct timespec req = {};

	common_errors(SELECT_CLOCK_NANOSLEEP, CLOCK_MONOTONIC, TIMER_ABSTIME);

	/* Absolute timeout in the past. */
	clock_gettime(CLOCK_MONOTONIC, &req);
	zassert_equal(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &req, &rem), 0);
	zassert_equal(rem.tv_sec, 0, "actual: %d expected: %d", (int)rem.tv_sec, 0);
	zassert_equal(rem.tv_nsec, 0, "actual: %d expected: %d", (int)rem.tv_nsec, 0);

	/* Absolute timeout in the past relative to the realtime clock. */
	clock_gettime(CLOCK_REALTIME, &req);
	zassert_equal(clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &req, &rem), 0);
	zassert_equal(rem.tv_sec, 0, "actual: %d expected: %d", (int)rem.tv_sec, 0);
	zassert_equal(rem.tv_nsec, 0, "actual: %d expected: %d", (int)rem.tv_nsec, 0);
}

static void common_abstime_accuracy(clockid_t clock_id)
{
	struct timespec now;
	struct timespec deadline;

	zassert_ok(clock_gettime(clock_id, &deadline));
	deadline.tv_nsec += 100 * NSEC_PER_MSEC;
	if (deadline.tv_nsec >= NSEC_PER_SEC) {
		deadline.tv_sec++;
		deadline.tv_nsec -= NSEC_PER_SEC;
	}

	zassert_ok(select_nanosleep(SELECT_CLOCK_NANOSLEEP, clock_id, TIMER_ABSTIME, &deadline,
				    NULL));

	/* the deadline is honored on the same clock it was specified against */
	zassert_ok(clock_gettime(clock_id, &now));
	zassert_true(timespec_compare(&now, &deadline) >= 0,
		     "clock_nanosleep() returned before the absolute deadline");
}

static void clock_nanosleep_abstime_accuracy(void)
{
	common_abstime_accuracy(CLOCK_MONOTONIC);
	common_abstime_accuracy(CLOCK_REALTIME);
}

#ifndef CONFIG_NATIVE_LIBC
static void premature_wakeup_fn(struct k_timer *timer)
{
	k_wakeup((k_tid_t)k_timer_user_data_get(timer));
}

K_TIMER_DEFINE(premature_wakeup_timer, premature_wakeup_fn, NULL);

static void clock_nanosleep_abstime_premature_wakeup(void)
{
	struct timespec now;
	struct timespec deadline;

	zassert_ok(clock_gettime(CLOCK_MONOTONIC, &deadline));
	deadline.tv_nsec += 100 * NSEC_PER_MSEC;
	if (deadline.tv_nsec >= NSEC_PER_SEC) {
		deadline.tv_sec++;
		deadline.tv_nsec -= NSEC_PER_SEC;
	}

	/* a premature k_wakeup() re-arms against the same absolute deadline */
	k_timer_user_data_set(&premature_wakeup_timer, k_current_get());
	k_timer_start(&premature_wakeup_timer, K_MSEC(25), K_NO_WAIT);

	zassert_ok(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL));

	zassert_ok(clock_gettime(CLOCK_MONOTONIC, &now));
	zassert_true(timespec_compare(&now, &deadline) >= 0,
		     "woken clock_nanosleep() returned before the absolute deadline");
}
#endif /* CONFIG_NATIVE_LIBC */

ZTEST(posix_clock_selection, test_clock_nanosleep)
{
	clock_nanosleep_errors_errno();
	clock_nanosleep_execution();
	clock_nanosleep_abstime_accuracy();
#ifndef CONFIG_NATIVE_LIBC
	/* a host-libc sleep cannot be interrupted by k_wakeup() */
	clock_nanosleep_abstime_premature_wakeup();
#endif
}
