/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* time and randomness */

#include <errno.h>
#include <string.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/clock.h>

#include "kcl_linux.h"

#define LINUX_CLOCK_REALTIME 0
#define LINUX_CLOCK_MONOTONIC 1
#define LINUX_CLOCK_PROCESS_CPUTIME_ID 2
#define LINUX_CLOCK_THREAD_CPUTIME_ID 3
#define LINUX_CLOCK_MONOTONIC_RAW 4
#define LINUX_CLOCK_REALTIME_COARSE 5
#define LINUX_CLOCK_MONOTONIC_COARSE 6
#define LINUX_CLOCK_BOOTTIME 7
#define LINUX_TIMER_ABSTIME 1

static int clock_of(int which, int *clock_id)
{
	switch (which) {
	case LINUX_CLOCK_REALTIME:
	case LINUX_CLOCK_REALTIME_COARSE:
		*clock_id = SYS_CLOCK_REALTIME;
		return 0;
	case LINUX_CLOCK_MONOTONIC:
	case LINUX_CLOCK_MONOTONIC_RAW:
	case LINUX_CLOCK_MONOTONIC_COARSE:
	case LINUX_CLOCK_BOOTTIME:
		*clock_id = SYS_CLOCK_MONOTONIC;
		return 0;
	default:
		return -EINVAL;
	}
}

ZKCL_LINUX_IMPL(getrandom)(char *buf, size_t count, unsigned int flags)
{
	ARG_UNUSED(flags);
	if (zkcl_linux_user_ok(buf, count, true) != 0) {
		return -EFAULT;
	}
	sys_rand_get(buf, count);

	return count;
}

ZKCL_LINUX_IMPL(clock_gettime)(zkcl_linux_clockid_t which_clock,
					     struct zkcl_linux_timespec *tp)
{
	struct timespec ts;
	struct zkcl_linux_timespec out;
	int clock_id;
	int ret;

	ret = clock_of(which_clock, &clock_id);
	if (ret != 0) {
		return ret;
	}
	if (zkcl_linux_user_ok(tp, sizeof(*tp), true) != 0) {
		return -EFAULT;
	}
	if ((clock_id == SYS_CLOCK_MONOTONIC) && IS_ENABLED(CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER)) {
		/* the cycle counter: the tick is too coarse for a round trip */
		uint64_t ns = k_cyc_to_ns_floor64(k_cycle_get_64());

		ts.tv_sec = ns / NSEC_PER_SEC;
		ts.tv_nsec = ns % NSEC_PER_SEC;
	} else {
		ret = sys_clock_gettime(clock_id, &ts);
		if (ret != 0) {
			return ret;
		}
	}
	out.tv_sec = ts.tv_sec;
	out.tv_nsec = ts.tv_nsec;
	memcpy(tp, &out, sizeof(out));

	return 0;
}

ZKCL_LINUX_IMPL(gettimeofday)(struct zkcl_linux_old_timeval *tv,
					    struct zkcl_linux_timezone *tz)
{
	struct timespec ts;
	struct zkcl_linux_old_timeval out;
	int ret;

	if (tz != NULL) {
		struct zkcl_linux_timezone zone = {0};

		if (zkcl_linux_user_ok(tz, sizeof(*tz), true) != 0) {
			return -EFAULT;
		}
		memcpy(tz, &zone, sizeof(zone));
	}
	if (tv == NULL) {
		return 0;
	}
	if (zkcl_linux_user_ok(tv, sizeof(*tv), true) != 0) {
		return -EFAULT;
	}
	ret = sys_clock_gettime(SYS_CLOCK_REALTIME, &ts);
	if (ret != 0) {
		return ret;
	}
	out.tv_sec = ts.tv_sec;
	out.tv_usec = ts.tv_nsec / 1000;
	memcpy(tv, &out, sizeof(out));

	return 0;
}

static long clock_nanosleep_impl(zkcl_linux_clockid_t which_clock, int flags,
					       const struct zkcl_linux_timespec *rqtp,
					       struct zkcl_linux_timespec *rmtp)
{
	struct zkcl_linux_timespec req;
	struct timespec ts;
	int clock_id;
	int ret;

	ARG_UNUSED(rmtp);
	ret = clock_of(which_clock, &clock_id);
	if (ret != 0) {
		return ret;
	}
	if (zkcl_linux_user_ok(rqtp, sizeof(*rqtp), false) != 0) {
		return -EFAULT;
	}
	memcpy(&req, rqtp, sizeof(req));
	if ((req.tv_nsec < 0) || (req.tv_nsec >= 1000000000LL)) {
		return -EINVAL;
	}
	ts.tv_sec = req.tv_sec;
	ts.tv_nsec = req.tv_nsec;

	return sys_clock_nanosleep(clock_id, (flags & LINUX_TIMER_ABSTIME) ? SYS_TIMER_ABSTIME : 0,
				   &ts, NULL);
}

ZKCL_LINUX_IMPL(clock_nanosleep)(zkcl_linux_clockid_t which_clock, int flags,
					       const struct zkcl_linux_timespec *rqtp,
					       struct zkcl_linux_timespec *rmtp)
{
	return clock_nanosleep_impl(which_clock, flags, rqtp, rmtp);
}

ZKCL_LINUX_IMPL(nanosleep)(struct zkcl_linux_timespec *rqtp,
					 struct zkcl_linux_timespec *rmtp)
{
	return clock_nanosleep_impl(LINUX_CLOCK_MONOTONIC, 0, rqtp, rmtp);
}

int zkcl_linux_user_ok(const void *ptr, size_t size, bool write)
{
	if (size == 0) {
		return 0;
	}
	if (((uintptr_t)ptr + size) < (uintptr_t)ptr) {
		return -EFAULT;
	}

	return (arch_buffer_validate(ptr, size, write) == 0) ? 0 : -EFAULT;
}

int zkcl_linux_user_string(char *dst, const char *src, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		/* a byte at a time: the string may end just before an unmapped page */
		if ((i == 0) || ((((uintptr_t)src + i) & (ZKCL_LINUX_PAGE_SIZE - 1)) == 0)) {
			size_t left = ZKCL_LINUX_PAGE_SIZE - (((uintptr_t)src + i) & (ZKCL_LINUX_PAGE_SIZE - 1));

			if (zkcl_linux_user_ok(src + i, MIN(left, size - i), false) != 0) {
				return -EFAULT;
			}
		}
		dst[i] = src[i];
		if (dst[i] == '\0') {
			return 0;
		}
	}

	return -ENAMETOOLONG;
}
