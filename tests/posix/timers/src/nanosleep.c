/*
 * Copyright (c) 2018 Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"

#define SELECT_NANOSLEEP       1
#define SELECT_CLOCK_NANOSLEEP 0

void common_lower_bound_check(int selection, clockid_t clock_id, int flags, const uint32_t s,
			      uint32_t ns);
int select_nanosleep(int selection, clockid_t clock_id, int flags, const struct timespec *rqtp,
		     struct timespec *rmtp);

static void common_errors(int selection, clockid_t clock_id, int flags)
{
	struct timespec rem = {};
	struct timespec req = {};

	/*
	 * invalid parameters (undefined behaviour with the host libc, which
	 * dereferences the request unconditionally)
	 */
	IF_NOT_NATIVE_LIBC({
		zassert_equal(select_nanosleep(selection, clock_id, flags, NULL, NULL), -1);
		zassert_equal(errno, EFAULT);

		/* NULL request */
		errno = 0;
		zassert_equal(select_nanosleep(selection, clock_id, flags, NULL, &rem), -1);
		zassert_equal(errno, EFAULT);
		/* Expect rem to be the same when function returns */
		zassert_equal(rem.tv_sec, 0, "actual: %d expected: %d", (int)rem.tv_sec, 0);
		zassert_equal(rem.tv_nsec, 0, "actual: %d expected: %d", (int)rem.tv_nsec, 0);
	})

	/* negative times */
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC) || (flags & TIMER_ABSTIME) == 0) {
		/* with the host libc, a negative absolute time-point is simply in the past */
		errno = 0;
		req = (struct timespec){.tv_sec = -1, .tv_nsec = 0};
		zassert_equal(select_nanosleep(selection, clock_id, flags, &req, NULL), -1);
		zassert_equal(errno, EINVAL);
	}

	errno = 0;
	req = (struct timespec){.tv_sec = 0, .tv_nsec = -1};
	zassert_equal(select_nanosleep(selection, clock_id, flags, &req, NULL), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	req = (struct timespec){.tv_sec = -1, .tv_nsec = -1};
	zassert_equal(select_nanosleep(selection, clock_id, flags, &req, NULL), -1);
	zassert_equal(errno, EINVAL);

	/* nanoseconds too high */
	errno = 0;
	req = (struct timespec){.tv_sec = 0, .tv_nsec = 1000000000};
	zassert_equal(select_nanosleep(selection, clock_id, flags, &req, NULL), -1);
	zassert_equal(errno, EINVAL);

	/*
	 * Valid parameters
	 */
	errno = 0;

	/* Happy path, plus make sure the const input is unmodified */
	req = (struct timespec){.tv_sec = 1, .tv_nsec = 1};
	zassert_equal(select_nanosleep(selection, clock_id, flags, &req, NULL), 0);
	zassert_equal(errno, 0);
	zassert_equal(req.tv_sec, 1);
	zassert_equal(req.tv_nsec, 1);

	/* Sleep for 0.0 s. Expect req & rem to be the same when function returns */
	zassert_equal(select_nanosleep(selection, clock_id, flags, &req, &rem), 0);
	zassert_equal(errno, 0);
	zassert_equal(rem.tv_sec, 0, "actual: %d expected: %d", (int)rem.tv_sec, 0);
	zassert_equal(rem.tv_nsec, 0, "actual: %d expected: %d", (int)rem.tv_nsec, 0);

	/*
	 * req and rem point to the same timespec
	 *
	 * Normative spec says they may be the same.
	 * Expect rem to be zero after returning.
	 */
	req = (struct timespec){.tv_sec = 0, .tv_nsec = 1};
	zassert_equal(select_nanosleep(selection, clock_id, flags, &req, &req), 0);
	zassert_equal(errno, 0);
	/* the host libc leaves rem untouched on success */
	IF_NOT_NATIVE_LIBC({
		zassert_equal(req.tv_sec, 0, "actual: %d expected: %d", (int)req.tv_sec, 0);
		zassert_equal(req.tv_nsec, 0, "actual: %d expected: %d", (int)req.tv_nsec, 0);
	})
}

static void nanosleep_errors_errno(void)
{
	common_errors(SELECT_NANOSLEEP, CLOCK_REALTIME, 0);
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

static void nanosleep_execution(void)
{
	/* sleep for 1ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 0, 1);

	/* sleep for 1us + 1ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 0, 1001);

	/* sleep for 500000000ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 0, 500000000);

	/* sleep for 1s */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 1, 0);

	/* sleep for 1s + 1ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 1, 1);

	/* sleep for 1s + 1us + 1ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 1, 1001);
}

ZTEST(posix_timers, test_nanosleep)
{
	nanosleep_errors_errno();
	nanosleep_execution();
}

ZTEST(posix_timers, test_clock_nanosleep)
{
	clock_nanosleep_errors_errno();
	clock_nanosleep_abstime_accuracy();
#ifndef CONFIG_NATIVE_LIBC
	/* a host-libc sleep cannot be interrupted by k_wakeup() */
	clock_nanosleep_abstime_premature_wakeup();
#endif
}
