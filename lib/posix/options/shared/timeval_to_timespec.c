/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>
#include <sys/time.h>

#include <zephyr/sys/clock.h>
#include <zephyr/sys/timeutil.h>

bool timeval_to_timespec(struct timeval *tv, struct timespec *ts)
{
	ts->tv_sec = tv->tv_sec;
	ts->tv_nsec = tv->tv_usec * NSEC_PER_USEC;

	return timespec_normalize(ts);
}
