/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX monetary formatting functions (<monetary.h>)
 *
 * Provides strfmon() and strfmon_l() for formatting monetary values according
 * to the current or a specified locale.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/monetary.h.html">
 *      POSIX.1-2017 &lt;monetary.h&gt;</a>
 *
 * @defgroup posix_monetary POSIX Monetary Formatting
 * @ingroup posix_option_group_c_lib_ext
 * @{
 */

#ifndef ZEPHYR_INCLUDE_POSIX_MONETARY_H_
#define ZEPHYR_INCLUDE_POSIX_MONETARY_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include <locale.h>
#include <stddef.h>
#include <sys/types.h>

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Format a monetary value into a string according to the current locale.
 * @param s       Output buffer.
 * @param maxsize Maximum number of bytes (including NUL) to write.
 * @param format  Conversion format string (%-based, similar to printf).
 * @param ...     Monetary values to format (type double).
 * @return Number of bytes written (excluding NUL) on success, or -1 on error.
 */
ssize_t strfmon(char *ZRESTRICT s, size_t maxsize, const char *ZRESTRICT format, ...);

/**
 * @brief Format a monetary value using the specified locale.
 * @param s       Output buffer.
 * @param maxsize Maximum number of bytes (including NUL) to write.
 * @param locale  Locale to use for formatting.
 * @param format  Conversion format string.
 * @param ...     Monetary values to format (type double).
 * @return Number of bytes written (excluding NUL) on success, or -1 on error.
 */
ssize_t strfmon_l(char *ZRESTRICT s, size_t maxsize, locale_t locale,
		  const char *ZRESTRICT format, ...);

#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

/** @} */

#endif /* ZEPHYR_INCLUDE_POSIX_MONETARY_H_ */
