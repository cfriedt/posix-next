/*
 * Copyright (c) 2018 Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>
#include <time.h>

#include <zephyr/sys_clock.h>
#include <zephyr/ztest.h>

#include "linux_compat_test.h"

#define SELECT_NANOSLEEP       1
#define SELECT_CLOCK_NANOSLEEP 0

static inline uint64_t cycle_get_64(void)
{
	if (IS_ENABLED(CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER)) {
		return k_cycle_get_64();
	} else {
		return k_cycle_get_32();
	}
}

int select_nanosleep(int selection, clockid_t clock_id, int flags, const struct timespec *rqtp,
		     struct timespec *rmtp)
{
	if (selection == SELECT_NANOSLEEP) {
		return nanosleep(rqtp, rmtp);
	}

	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the host libc returns the error number instead of setting errno */
		int ret = clock_nanosleep(clock_id, flags, rqtp, rmtp);

		if (ret != 0) {
			errno = ret;
			return -1;
		}

		return 0;
	}

	return clock_nanosleep(clock_id, flags, rqtp, rmtp);
}

/*
 * With the host libc, sleeps happen in host time while the cycle counter tracks simulated
 * time, so time-points must be sampled from the same clock the sleep is measured against.
 */
static uint64_t timepoint_ns(int selection, clockid_t clock_id)
{
#if defined(CONFIG_NATIVE_LIBC)
	struct timespec tp;
	const clockid_t meas_clock =
		(selection == SELECT_CLOCK_NANOSLEEP) ? clock_id : CLOCK_MONOTONIC;

	zassert_ok(clock_gettime(meas_clock, &tp));

	return (uint64_t)tp.tv_sec * NSEC_PER_SEC + tp.tv_nsec;
#else
	ARG_UNUSED(selection);
	ARG_UNUSED(clock_id);

	return cycle_get_64();
#endif
}

/**
 * @brief Check that a call to nanosleep has yielded execution for some minimum time.
 *
 * Check that the actual time slept is >= the total time specified by @p s (in seconds) and
 * @p ns (in nanoseconds).
 *
 * @note The time specified by @p s and @p ns is assumed to be absolute (i.e. a time-point)
 * when @p selection is set to @ref SELECT_CLOCK_NANOSLEEP. The time is assumed to be relative
 * when @p selection is set to @ref SELECT_NANOSLEEP.
 *
 * @param selection Either @ref SELECT_CLOCK_NANOSLEEP or @ref SELECT_NANOSLEEP
 * @param clock_id The clock to test (e.g. @ref CLOCK_MONOTONIC or @ref CLOCK_REALTIME)
 * @param flags Flags to pass to @ref clock_nanosleep
 * @param s Partial lower bound for yielded time (in seconds)
 * @param ns Partial lower bound for yielded time (in nanoseconds)
 */
void common_lower_bound_check(int selection, clockid_t clock_id, int flags, const uint32_t s,
			      uint32_t ns)
{
	int r;
	uint64_t actual_ns = 0;
	uint64_t exp_ns;
	uint64_t now;
	uint64_t then;
	struct timespec rem = {0, 0};
	struct timespec req = {s, ns};

	errno = 0;
	then = timepoint_ns(selection, clock_id);
	r = select_nanosleep(selection, clock_id, flags, &req, &rem);
	now = timepoint_ns(selection, clock_id);

	zassert_equal(r, 0, "actual: %d expected: %d", r, 0);
	zassert_equal(errno, 0, "actual: %d expected: %d", errno, 0);
	zassert_equal(req.tv_sec, s, "actual: %lld expected: %d", (long long)req.tv_sec, s);
	zassert_equal(req.tv_nsec, ns, "actual: %lld expected: %d", (long long)req.tv_nsec, ns);
	zassert_equal(rem.tv_sec, 0, "actual: %lld expected: %d", (long long)rem.tv_sec, 0);
	zassert_equal(rem.tv_nsec, 0, "actual: %lld expected: %d", (long long)rem.tv_nsec, 0);

	switch (selection) {
	case SELECT_NANOSLEEP:
		/* exp_ns and actual_ns are relative (i.e. durations) */
		if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
			actual_ns = now - then;
		} else {
			actual_ns = k_cyc_to_ns_ceil64(now + then);
		}
		break;
	case SELECT_CLOCK_NANOSLEEP:
		/* exp_ns and actual_ns are absolute (i.e. time-points) */
		if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
			actual_ns = now;
		} else {
			actual_ns = k_cyc_to_ns_ceil64(now);
		}
		break;
	default:
		zassert_unreachable();
		break;
	}

	exp_ns = (uint64_t)s * NSEC_PER_SEC + ns;
	/* round up to the nearest microsecond for k_busy_wait() */
	exp_ns = DIV_ROUND_UP(exp_ns, NSEC_PER_USEC) * NSEC_PER_USEC;

/* The comparison may be incorrect if counter wrap happened. In case of ARC HSDK platforms
 * we have high counter clock frequency (500MHz or 1GHz) so counter wrap quite likely to
 * happen if we wait long enough. As in some test cases we wait more than 1 second, there
 * are significant chances to get false-positive assertion.
 * TODO: switch test for k_cycle_get_64 usage where available.
 */
#if !defined(CONFIG_SOC_ARC_HSDK) && !defined(CONFIG_SOC_ARC_HSDK4XD)
	/* lower bounds check */
	zassert_true(actual_ns >= exp_ns, "actual: %llu expected: %llu", actual_ns, exp_ns);
#endif

	/* TODO: Upper bounds check when hr timers are available */
}

void common_errors(int selection, clockid_t clock_id, int flags)
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
