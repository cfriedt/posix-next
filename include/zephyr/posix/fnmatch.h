/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX filename pattern matching (<fnmatch.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/fnmatch.h.html">
 *      POSIX.1-2017 &lt;fnmatch.h&gt;</a>
 *
 */

#ifndef ZEPHYR_INCLUDE_POSIX_FNMATCH_H_
#define ZEPHYR_INCLUDE_POSIX_FNMATCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Return value of fnmatch() indicating the string did not match the pattern.  @ingroup posix_option_group_c_lib_ext*/
#undef FNM_NOMATCH
#define FNM_NOMATCH  1
/** @brief Flag: treat backslash as an ordinary character rather than an escape character.  @ingroup posix_option_group_c_lib_ext*/
#undef FNM_NOESCAPE
#define FNM_NOESCAPE 0x01
/** @brief Flag: a slash must be matched by a slash (not '?' or '*' or a bracket expression).  @ingroup posix_option_group_c_lib_ext*/
#undef FNM_PATHNAME
#define FNM_PATHNAME 0x02
/** @brief Flag: a leading '.' must be matched explicitly.  @ingroup posix_option_group_c_lib_ext*/
#undef FNM_PERIOD
#define FNM_PERIOD   0x04
#if defined(_GNU_SOURCE)
/** @brief GNU extension: match a leading directory portion (stops at '/').  @ingroup posix_option_group_c_lib_ext*/
#undef FNM_LEADING_DIR
#define FNM_LEADING_DIR 0x08
/** @brief GNU extension: case-insensitive matching.  @ingroup posix_option_group_c_lib_ext*/
#undef FNM_CASEFOLD
#define FNM_CASEFOLD    0x10
/** @brief GNU extension: extended pattern matching (?, *, +, @, ! prefixes).  @ingroup posix_option_group_c_lib_ext*/
#undef FNM_EXTMATCH
#define FNM_EXTMATCH    0x20
#endif
/** @brief Alias for FNM_CASEFOLD.  @ingroup posix_option_group_c_lib_ext*/
#undef FNM_IGNORECASE
#define FNM_IGNORECASE FNM_CASEFOLD

/**
 * @brief Match a filename or path against a shell-style pattern.
 * @ingroup posix_option_group_c_lib_ext
 * @param pattern Shell pattern (using ?, *, and bracket expressions).
 * @param string  String to test against @p pattern.
 * @param flags   Combination of FNM_* flags, or 0 for default matching.
 * @return 0 if @p string matches @p pattern, @c FNM_NOMATCH if it does not,
 *         or a non-zero value on error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fnmatch.html
 */
int fnmatch(const char *pattern, const char *string, int flags);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_FNMATCH_H_ */
