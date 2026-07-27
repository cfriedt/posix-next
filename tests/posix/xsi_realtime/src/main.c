/*
 * Copyright (c) 2025 Marvin Ouma <pancakesdeath@protonmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#include <zephyr/ztest.h>

ZTEST(xsi_realtime, test_sched_getparam)
{
	struct sched_param param = {.sched_priority = -1};

	/* pid 0 designates the calling process (the calling thread) */
	zassert_ok(sched_getparam(0, &param));
	zassert_true(param.sched_priority >= 0);

	/* no other process exists */
	zassert_equal(-1, sched_getparam(INT_MAX, &param));
	zassert_equal(errno, ESRCH);
}

ZTEST(xsi_realtime, test_sched_getscheduler)
{
	int policy = sched_getscheduler(0);

	zassert_true(policy == SCHED_FIFO || policy == SCHED_RR || policy == SCHED_OTHER,
		     "unexpected policy %d", policy);
}
ZTEST(xsi_realtime, test_sched_setparam)
{
	struct sched_param param;

	/* re-applying the current priority is always permitted */
	zassert_ok(sched_getparam(0, &param));
	zassert_ok(sched_setparam(0, &param));

	zassert_equal(-1, sched_setparam(INT_MAX, &param));
	zassert_equal(errno, ESRCH);
}

ZTEST(xsi_realtime, test_sched_setscheduler)
{
	struct sched_param param;
	int policy = sched_getscheduler(0);

	zassert_ok(sched_getparam(0, &param));

	/*
	 * Re-applying the current policy and priority succeeds. POSIX
	 * specifies the former policy as the return value; Linux returns 0,
	 * so accept any non-negative result.
	 */
	zassert_true(sched_setscheduler(0, policy, &param) >= 0);

	zassert_equal(-1, sched_setscheduler(INT_MAX, policy, &param));
	zassert_equal(errno, ESRCH);
}

ZTEST(xsi_realtime, test_sched_rr_get_interval)
{
	struct timespec interval = {
		.tv_sec = -1,
		.tv_nsec = -1,
	};

	zassert_ok(sched_rr_get_interval(0, &interval));
	zassert_true(interval.tv_sec >= 0);
	zassert_true(interval.tv_nsec >= 0 && interval.tv_nsec < NSEC_PER_SEC);

	zassert_equal(-1, sched_rr_get_interval(INT_MAX, &interval));
	zassert_equal(errno, ESRCH);
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);

	if (IS_ENABLED(CONFIG_COVERAGE)) {
		extern int usleep(useconds_t usec);
		/* Wait a few seconds before main() exit, giving the sample the
		 * opportunity to dump some output before coverage data gets emitted
		 */
		usleep(5 * USEC_PER_SEC);
	}
}

ZTEST_SUITE(xsi_realtime, NULL, NULL, NULL, NULL, teardown);
