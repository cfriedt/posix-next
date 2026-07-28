/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* for SIGEV_THREAD_ID, where the libc provides it */
#define _GNU_SOURCE

#include "posix_clock.h"
#include "posix_internal.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/timer.h>

/*
 * timer_t is the struct k_timer pointer itself. In user mode, stale or foreign handles are
 * rejected by kernel object validation in every syscall; in kernel mode, sys_timer_delete()
 * validates pool membership.
 *
 * The only module-side state is the SIGEV_THREAD helper context, linked via the kernel
 * timer's user data. All arming, timing, and overrun state lives kernel-side.
 */

static ALWAYS_INLINE struct k_timer *to_timer(timer_t timerid)
{
	return (struct k_timer *)(uintptr_t)timerid;
}

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
