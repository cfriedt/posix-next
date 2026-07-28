/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_POSIX_CLOCK_H_
#define ZEPHYR_LIB_POSIX_POSIX_CLOCK_H_

#include <errno.h>
#include <time.h>

#include <zephyr/sys/clock.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @cond INTERNAL_HIDDEN */

/* Convert a POSIX clock (cast to int) to a sys_clock identifier */
static inline int sys_clock_from_clockid(int clock_id)
{
	switch (clock_id) {
	case (int)CLOCK_REALTIME:
		return SYS_CLOCK_REALTIME;
#if defined(_POSIX_MONOTONIC_CLOCK)
	case (int)CLOCK_MONOTONIC:
		return SYS_CLOCK_MONOTONIC;
#endif
	default:
		return -EINVAL;
	}
}

/** INTERNAL_HIDDEN @endcond */

#ifdef __cplusplus
}
#endif

#endif
