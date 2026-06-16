/*
 * Copyright (c) 2025 The Zephyr Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief \<stdlib.h\>: POSIX extensions to the C standard library
 *
 * Provides POSIX and XSI extensions to the standard @c <stdlib.h> interface,
 * including environment variable manipulation and sub-option parsing.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdlib.h.html">
 *      POSIX.1-2017 &lt;stdlib.h&gt;</a>
 *
 */

#ifndef ZEPHYR_INCLUDE_POSIX_POSIX_STDLIB_H_
#define ZEPHYR_INCLUDE_POSIX_POSIX_STDLIB_H_

#include <stddef.h> /* NULL, size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: additional POSIX signatures here */

#if defined(_BSD_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Get an environment variable into a caller-supplied buffer (BSD extension).
 *
 * @param name Name of the environment variable.
 * @param buf  Buffer to write the value into.
 * @param len  Size of @p buf.
 * @return 0 on success, or a positive error number on failure (ENOENT if not set,
 *         ERANGE if @p buf is too small).
 */
int getenv_r(const char *name, char *buf, size_t len);
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Parse sub-options from a command-line argument string.
 * @ingroup posix_option_group_c_lib_ext
 *
 * Modifies @p *optionp in place, separating comma-delimited sub-options.
 *
 * @param optionp  Pointer to the option string; advanced past each parsed token.
 * @param keylistp NULL-terminated array of recognised keyword strings.
 * @param valuep   Output: pointer to the value part after '=', or NULL.
 * @return Index into @p keylistp of the matched keyword, or -1 if not found.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getsubopt.html
 */
int getsubopt(char **optionp, char *const *keylistp, char **valuep);
#endif

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Add or change an environment variable (XSI extension).
 * @ingroup posix_option_group_xsi_single_process
 *
 * @p string must be of the form @c "NAME=value".  The string is placed
 * directly in the environment, so it must remain valid for the lifetime of
 * the process.
 *
 * @param string Environment entry of the form "NAME=value".
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/putenv.html
 */
int putenv(char *string);
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Set the value of an environment variable.
 * @ingroup posix_option_group_single_process
 * @param envname   Name of the variable (must not contain '=').
 * @param envval    New value string.
 * @param overwrite Non-zero to overwrite an existing value; 0 to preserve it.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setenv.html
 */
int setenv(const char *envname, const char *envval, int overwrite);

/**
 * @brief Remove an environment variable.
 * @ingroup posix_option_group_single_process
 * @param name Name of the variable to remove.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/unsetenv.html
 */
int unsetenv(const char *name);
#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_POSIX_STDLIB_H_ */
