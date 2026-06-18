/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX string operations (<strings.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/strings.h.html">
 *      POSIX.1-2017 &lt;strings.h&gt;</a>
 *
 * @ingroup posix_option_group_c_lib_ext
 */

#ifndef ZEPHYR_INCLUDE_POSIX_STRINGS_H_
#define ZEPHYR_INCLUDE_POSIX_STRINGS_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compare two strings, ignoring case.
 * @ingroup posix_option_group_c_lib_ext
 * @param s1 First string.
 * @param s2 Second string.
 * @return An integer less than, equal to, or greater than zero if @p s1 is found,
 *         respectively, to be less than, equal to, or greater than @p s2.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/strcasecmp.html
 */
int strcasecmp(const char *s1, const char *s2);

/**
 * @brief Compare two strings, ignoring case, up to a maximum length.
 * @ingroup posix_option_group_c_lib_ext
 * @param s1 First string.
 * @param s2 Second string.
 * @param n Maximum number of bytes to compare.
 * @return An integer less than, equal to, or greater than zero if the first @p n bytes
 *         of @p s1 are found, respectively, to be less than, equal to, or greater than
 *         those of @p s2.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/strncasecmp.html
 */
int strncasecmp(const char *s1, const char *s2, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif /* ZEPHYR_INCLUDE_POSIX_STRINGS_H_ */
