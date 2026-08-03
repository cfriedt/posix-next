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

#include "../../shared/linux_compat_test.h"
#include "../../shared/nanosleep_common.h"

ZTEST_USER(posix_clock_selection, test_pthread_condattr_getclock)
{
	clockid_t clock_id;
	pthread_condattr_t att = {0};

	zassert_ok(pthread_condattr_init(&att));

	zassert_ok(pthread_condattr_getclock(&att, &clock_id), "pthread_condattr_getclock failed");
	zassert_equal(clock_id, CLOCK_REALTIME, "clock attribute not set correctly");

	zassert_ok(pthread_condattr_destroy(&att));
}
