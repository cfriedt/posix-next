/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_clock.h"
#include "posix_internal.h"
#include "timers_internal.h"

#include <errno.h>
#include <signal.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/timer.h>

int timer_settime(timer_t timerid, int flags, const struct itimerspec *value,
		  struct itimerspec *ovalue)
{
	int ret;
	struct k_timer *const t = to_timer(timerid);
	const uint32_t kflags = ((flags & TIMER_ABSTIME) != 0) ? SYS_TIMER_ABSTIME : 0U;

	if ((t == NULL) || (value == NULL)) {
		errno = EINVAL;
		return -1;
	}

	ret = k_timer_settime(t, kflags, &value->it_value, &value->it_interval,
			      (ovalue == NULL) ? NULL : &ovalue->it_value,
			      (ovalue == NULL) ? NULL : &ovalue->it_interval);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}
