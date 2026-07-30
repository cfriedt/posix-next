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

int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid)
{
	int ret;
	uint32_t flags = 0;
	struct k_timer *t = NULL;
	struct k_timer_notify notify;
	struct k_timer_notify *notifyp = NULL;
	struct sigevent default_event;
	const int clock_id = sys_clock_from_clockid((int)clockid);

	if ((timerid == NULL) || (clock_id < 0)) {
		errno = EINVAL;
		return -1;
	}

	if (evp == NULL) {
		/*
		 * POSIX default: as if SIGEV_SIGNAL with signo SIGALRM and the timer ID as
		 * the value; the kernel stamps the value at attach via SYS_TIMER_SIGVAL_SELF.
		 */
		default_event = (struct sigevent){
			.sigev_notify = SIGEV_SIGNAL,
			.sigev_signo = SIGALRM,
		};
		evp = &default_event;
		flags |= SYS_TIMER_SIGVAL_SELF;
	}

	ret = posix_sigev_validate(evp, true);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	/* for SIGEV_THREAD, starts the singleton watcher on first use */
	ret = posix_sigev_to_notify(evp, &notify);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	if (ret > 0) {
		notifyp = &notify;
	}

	/* created disarmed, per POSIX; timer_settime() arms */
	ret = sys_timer_create(clock_id, notifyp, flags, NULL, NULL, &t);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	*timerid = (timer_t)(uintptr_t)t;

	return 0;
}
