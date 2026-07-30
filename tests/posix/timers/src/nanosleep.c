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
void common_errors(int selection, clockid_t clock_id, int flags);
int select_nanosleep(int selection, clockid_t clock_id, int flags, const struct timespec *rqtp,
		     struct timespec *rmtp);

static void nanosleep_errors_errno(void)
{
	common_errors(SELECT_NANOSLEEP, CLOCK_REALTIME, 0);
}

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

