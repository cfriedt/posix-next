/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sched.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_sporadic_server, test_sched_get_priority_max)
{
	int min = sched_get_priority_min(SCHED_SPORADIC);
	int max = sched_get_priority_max(SCHED_SPORADIC);

	zassert_true(max >= 0, "sched_get_priority_max(SCHED_SPORADIC) returned %d", max);
	zassert_true(max >= min, "max (%d) < min (%d) for SCHED_SPORADIC", max, min);
}
