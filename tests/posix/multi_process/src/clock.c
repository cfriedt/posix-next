/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

ZTEST(posix_multi_process, test_clock)
{
	clock_t before;
	clock_t after;

	before = clock();
	k_busy_wait(USEC_PER_MSEC);
	after = clock();

	/* clock() returns (clock_t)-1 when processor time is unavailable */
	zassert_true(after >= before, "clock() went backwards: %ld -> %ld", (long)before,
		     (long)after);
}
