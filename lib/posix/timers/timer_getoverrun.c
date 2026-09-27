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

int timer_getoverrun(timer_t timerid)
{
	struct k_timer *const t = to_timer(timerid);

	if (t == NULL) {
		errno = EINVAL;
		return -1;
	}

#ifdef CONFIG_TIMER_SIGNAL
	return MIN(k_timer_overrun(t), CONFIG_POSIX_DELAYTIMER_MAX);
#else
	return 0;
#endif /* CONFIG_TIMER_SIGNAL */
}
