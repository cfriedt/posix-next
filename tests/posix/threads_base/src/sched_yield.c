/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sched.h>

#include <zephyr/ztest.h>

#include "_main.h"

static void test_sched_yield(void)
{
	zassert_ok(sched_yield());
}

ZTEST_THREADS_BASE(test_sched_yield);
