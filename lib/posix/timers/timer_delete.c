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

int timer_delete(timer_t timerid)
{
	int ret;
	struct k_timer *const t = to_timer(timerid);

	if (t == NULL) {
		errno = EINVAL;
		return -1;
	}

	/* stops the timer, detaches notification, and purges queued expiry signals */
	ret = sys_timer_delete(t);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}
