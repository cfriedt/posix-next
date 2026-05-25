/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX time-of-day types and functions (<sys/time.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_time.h.html">
 *      POSIX.1-2017 &lt;sys/time.h&gt;</a>
 *
 * @ingroup posix_option_group_xsi_single_process
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_TIME_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_TIME_H_

#ifdef CONFIG_NEWLIB_LIBC
/* Kludge to support outdated newlib version as used in SDK 0.10 for Xtensa */
#include <newlib.h>

#ifdef __NEWLIB__
#include <sys/_timeval.h>
#else
#include <sys/types.h>
struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;
};
#endif

#else
#include <sys/types.h>
#include <sys/_timeval.h>
#endif /* CONFIG_NEWLIB_LIBC */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the current time with microsecond resolution.
 * @ingroup posix_option_group_xsi_single_process
 *
 * @param tv Output: current time; @c tv_sec is seconds and @c tv_usec is microseconds
 *           since the Epoch.
 * @param tz Deprecated, must be NULL.
 * @return 0 on success, or -1 with errno set on failure.
 */
int gettimeofday(struct timeval *tv, void *tz);


#ifdef __cplusplus
}
#endif

#endif	/* ZEPHYR_INCLUDE_POSIX_SYS_TIME_H_ */
