/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timers_tests.h"

#include <zephyr/sys/timeutil.h>

#define CLOCK_INVALID -1

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(timer_test);

static const clockid_t clocks[] = {
#if defined(_POSIX_MONOTONIC_CLOCK)
	CLOCK_MONOTONIC,
#endif
	CLOCK_REALTIME,
};

static void clock_gettime_supported_clocks(void)
{
	struct timespec ts;

	/* ensure argument validation is performed */
	errno = 0;
	zassert_equal(clock_gettime(CLOCK_INVALID, &ts), -1);
	zassert_equal(errno, EINVAL);

	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			errno = 0;
			zassert_equal(clock_gettime(clocks[0], NULL), -1);
			zassert_equal(errno, EINVAL);
		}
	})

	/* verify that we can call clock_gettime() on supported clocks */
	ARRAY_FOR_EACH(clocks, i) {
		ts = (struct timespec){-1, -1};
		zassert_ok(clock_gettime(clocks[i], &ts));
		zassert_not_equal(ts.tv_sec, -1);
		zassert_not_equal(ts.tv_nsec, -1);
	}
}

static void clock_gettime_realtime_progression(void)
{
	struct timespec diff;
	struct timespec then, now;
	/*
	 * For calculating cumulative moving average
	 * Note: we do not want to assert any individual samples due to scheduler noise.
	 * The CMA filters out the noise so we can make an assertion (on average).
	 * https://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
	 */
	int64_t cma_prev = 0;
	int64_t cma;
	int64_t x_i;
	/* lower and uppoer boundary for assertion */
	int64_t lo = CONFIG_TEST_CLOCK_RT_SLEEP_MS;
	int64_t hi = CONFIG_TEST_CLOCK_RT_SLEEP_MS + CONFIG_TEST_CLOCK_RT_ERROR_MS;
	/* lower and upper watermark */
	int64_t lo_wm = INT64_MAX;
	int64_t hi_wm = INT64_MIN;

	/* Loop n times, sleeping a little bit for each */
	(void)clock_gettime(CLOCK_REALTIME, &then);
	for (int i = 0; i < CONFIG_TEST_CLOCK_RT_ITERATIONS; ++i) {

		if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
			/*
			 * Simulated time may lag real time, in which case k_usleep() would
			 * complete without any real time elapsing on the host clock.
			 */
			zassert_ok(usleep(USEC_PER_MSEC * CONFIG_TEST_CLOCK_RT_SLEEP_MS));
		} else {
			zassert_ok(k_usleep(USEC_PER_MSEC * CONFIG_TEST_CLOCK_RT_SLEEP_MS));
		}
		(void)clock_gettime(CLOCK_REALTIME, &now);

		/* Make the delta milliseconds. */
		diff = now;
		(void)timespec_sub(&diff, &then);
		x_i = (int64_t)diff.tv_sec * MSEC_PER_SEC + diff.tv_nsec / NSEC_PER_MSEC;
		then = now;

		if (x_i < lo_wm) {
			/* update low watermark */
			lo_wm = x_i;
		}

		if (x_i > hi_wm) {
			/* update high watermark */
			hi_wm = x_i;
		}

		/* compute cumulative running average */
		cma = (x_i + i * cma_prev) / (i + 1);
		cma_prev = cma;
	}

	LOG_INF("n: %d, sleep: %d, margin: %d, lo: %lld, avg: %lld, hi: %lld",
		CONFIG_TEST_CLOCK_RT_ITERATIONS, CONFIG_TEST_CLOCK_RT_SLEEP_MS,
		CONFIG_TEST_CLOCK_RT_ERROR_MS, lo_wm, cma, hi_wm);
	zassert_between_inclusive(cma, lo, hi);
}

ZTEST(posix_timers, test_clock_gettime)
{
	clock_gettime_supported_clocks();
	clock_gettime_realtime_progression();
}
