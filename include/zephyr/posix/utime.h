/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX file access and modification times (<utime.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/utime.h.html">
 *      POSIX.1-2017 &lt;utime.h&gt;</a>
 *
 * @ingroup posix_option_group_file_system
 */

#ifndef ZEPHYR_INCLUDE_POSIX_UTIME_H_
#define ZEPHYR_INCLUDE_POSIX_UTIME_H_

/* time_t must be provided by the libc <time.h> */
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#if !defined(_UTIMBUF_DECLARED) && !defined(__utimbuf_defined)
/**
 * @brief File times passed to utime().
 * @ingroup posix_option_group_file_system
 */
struct utimbuf {
	time_t actime;  /**< Access time. */
	time_t modtime; /**< Modification time. */
};
#define _UTIMBUF_DECLARED
#define __utimbuf_defined
#endif

/**
 * @brief Set file access and modification times.
 * @ingroup posix_option_group_file_system
 * @deprecated Obsolescent in POSIX.1-2017 and removed from POSIX_FILE_SYSTEM
 *             in POSIX.1-2024; use utimensat() instead.
 * @param path Path of the file or directory.
 * @param times Access and modification times, or NULL to set both to the
 *              current time.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/utime.html
 */
int utime(const char *path, const struct utimbuf *times);

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_UTIME_H_ */
