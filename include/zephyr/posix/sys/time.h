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

#include <sys/select.h>

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
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/gettimeofday.html
 */
int gettimeofday(struct timeval *tv, void *tz);

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Set file access and modification times (obsolescent).
 * @ingroup posix_option_group_xsi_file_system
 *
 * @param path Path to the file.
 * @param times Access and modification times, or NULL for the current time.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/utimes.html
 */
int utimes(const char *path, const struct timeval times[2]);

/**
 * @brief Set the current time (legacy; removed from POSIX.1-2001).
 * @ingroup posix_option_group_xsi_single_process
 *
 * @param tv New current time.
 * @param tz Must be NULL.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/7908799/xsh/settimeofday.html
 */
int settimeofday(const struct timeval *tv, const void *tz);
#endif /* _XOPEN_SOURCE || __DOXYGEN__ */

#ifdef __cplusplus
}
#endif

#endif	/* ZEPHYR_INCLUDE_POSIX_SYS_TIME_H_ */
