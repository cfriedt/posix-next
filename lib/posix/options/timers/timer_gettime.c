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

int timer_gettime(timer_t timerid, struct itimerspec *value)
{
	int ret;
	struct k_timer *const t = to_timer(timerid);

	if ((t == NULL) || (value == NULL)) {
		errno = EINVAL;
		return -1;
	}

	ret = k_timer_gettime(t, &value->it_value, &value->it_interval);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}
