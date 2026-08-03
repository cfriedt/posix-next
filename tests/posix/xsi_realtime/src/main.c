/*
 * Copyright (c) 2025 Marvin Ouma <pancakesdeath@protonmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>
#include <time.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/*
 * Zephyr's process-addressed scheduling functions are ENOSYS stubs. The host
 * libc implements them: a query succeeds, and a non-zero priority is invalid
 * for the SCHED_OTHER process the test runs as.
 */
#define NATIVE (IS_ENABLED(CONFIG_NATIVE_LIBC))

ZTEST(xsi_realtime, test_sched_getparam)
{
	struct sched_param param;
	int rc = sched_getparam(0, &param);
	int err = errno;

	if (NATIVE) {
		zassert_ok(rc, "sched_getparam() failed: %d", err);
	} else {
		zassert_true((rc == -1 && err == ENOSYS));
	}
}

ZTEST(xsi_realtime, test_sched_getscheduler)
{
	int rc = sched_getscheduler(0);
	int err = errno;

	if (NATIVE) {
		zassert_true(rc >= 0, "sched_getscheduler() failed: %d", err);
	} else {
		zassert_true((rc == -1 && err == ENOSYS));
	}
}
ZTEST(xsi_realtime, test_sched_setparam)
{
	struct sched_param param = {
		.sched_priority = 2,
	};
	int rc = sched_setparam(0, &param);
	int err = errno;

	zassert_true((rc == -1 && err == (NATIVE ? EINVAL : ENOSYS)), "rc %d errno %d", rc, err);
}

ZTEST(xsi_realtime, test_sched_setscheduler)
{
	struct sched_param param = {
		.sched_priority = 2,
	};
	int policy = 0;
	int rc = sched_setscheduler(0, policy, &param);
	int err = errno;

	zassert_true((rc == -1 && err == (NATIVE ? EINVAL : ENOSYS)), "rc %d errno %d", rc, err);
}

ZTEST(xsi_realtime, test_sched_rr_get_interval)
{
	struct timespec interval = {
		.tv_sec = 0,
		.tv_nsec = 0,
	};
	int rc = sched_rr_get_interval(0, &interval);
	int err = errno;

	if (NATIVE) {
		zassert_ok(rc, "sched_rr_get_interval() failed: %d", err);
	} else {
		zassert_true((rc == -1 && err == ENOSYS));
	}
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);

	if (IS_ENABLED(CONFIG_COVERAGE)) {
		/* leave the output a few seconds to drain before the coverage dump */
		const struct timespec ts = {.tv_sec = 5};

		(void)nanosleep(&ts, NULL);
	}
}

ZTEST_SUITE(xsi_realtime, NULL, NULL, NULL, NULL, teardown);
