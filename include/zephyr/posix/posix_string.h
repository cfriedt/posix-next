/*
 * Copyright (c) 2025 The Zephyr Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX extensions to the C string library (<string.h> supplement)
 *
 * Provides POSIX extensions to the standard @c <string.h> interface, including
 * thread-safe and signal-related string utilities.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/string.h.html">
 *      POSIX.1-2017 &lt;string.h&gt;</a>
 *
 * @ingroup posix_option_group_signals
 */

#ifndef ZEPHYR_INCLUDE_POSIX_POSIX_STRING_H_
#define ZEPHYR_INCLUDE_POSIX_POSIX_STRING_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include <stddef.h> /* NULL, size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: additional POSIX signatures here */

/**
 * @brief Return a string describing a signal number.
 * @ingroup posix_option_group_signals
 *
 * Returns a pointer to a locale-specific message string describing the signal
 * whose number is @p signo.  The contents of the string pointed to by the
 * return value must not be modified by the application.  The returned pointer
 * may be invalidated by a subsequent call to strsignal().
 *
 * @param signo Signal number.
 * @return Pointer to a string describing the signal, or an unspecified string
 *         if @p signo is an unknown signal number.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/strsignal.html
 */
char *strsignal(int signo);


#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif /* ZEPHYR_INCLUDE_POSIX_POSIX_STRING_H_ */
