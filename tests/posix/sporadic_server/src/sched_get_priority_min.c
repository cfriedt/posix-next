/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sched.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_sporadic_server, test_sched_get_priority_min)
{
	int min = sched_get_priority_min(SCHED_SPORADIC);

	zassert_true(min >= 0, "sched_get_priority_min(SCHED_SPORADIC) returned %d", min);
}
