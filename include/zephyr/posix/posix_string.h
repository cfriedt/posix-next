/*
 * Copyright (c) 2025 The Zephyr Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief \<string.h\>: POSIX extensions to the C string library
 *
 * Provides POSIX extensions to the standard @c <string.h> interface that are
 * not part of ISO C.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/string.h.html">
 *      POSIX.1-2017 &lt;string.h&gt;</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_POSIX_STRING_H_
#define ZEPHYR_INCLUDE_POSIX_POSIX_STRING_H_

#include <stddef.h> /* NULL, size_t */

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Copy a string and return a pointer to the terminating NUL byte.
 * @ingroup posix_option_group_c_lib_ext
 * @param s1 Destination buffer.
 * @param s2 Source string.
 * @return Pointer to the terminating NUL byte written to @p s1.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/stpcpy.html
 */
char *stpcpy(char *ZRESTRICT s1, const char *ZRESTRICT s2);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Copy at most @p n bytes of a string and return a pointer to the terminating NUL byte.
 * @ingroup posix_option_group_c_lib_ext
 * @param s1 Destination buffer.
 * @param s2 Source string.
 * @param n Maximum number of bytes to copy from @p s2.
 * @return Pointer to the terminating NUL byte written to @p s1, or @c NULL if @p n is zero.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/stpncpy.html
 */
char *stpncpy(char *ZRESTRICT s1, const char *ZRESTRICT s2, size_t n);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 500) || defined(__DOXYGEN__)
/**
 * @brief Duplicate a string.
 * @ingroup posix_option_group_c_lib_ext
 * @param s Source string.
 * @return Pointer to newly allocated memory containing the duplicate, or @c NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/strdup.html
 */
char *strdup(const char *s);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 500) || defined(__DOXYGEN__) */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Duplicate at most @p n bytes of a string.
 * @ingroup posix_option_group_c_lib_ext
 * @param s Source string.
 * @param n Maximum number of bytes to copy from @p s.
 * @return Pointer to newly allocated memory containing the duplicate, or @c NULL
 *         on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/strndup.html
 */
char *strndup(const char *s, size_t n);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Determine the length of a fixed-size string.
 * @ingroup posix_option_group_c_lib_ext
 * @param s Source string.
 * @param maxlen Maximum number of bytes to examine.
 * @return Number of bytes in @p s, excluding the terminating NUL byte, if that
 *         number is less than @p maxlen; otherwise @p maxlen.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/strnlen.html
 */
size_t strnlen(const char *s, size_t maxlen);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Tokenize a string into tokens (reentrant).
 * @ingroup posix_option_group_c_lang_support_r
 * @param str First call passes the string; subsequent calls pass NULL.
 * @param sep Delimiter characters.
 * @param state Caller-provided state for reentrant tokenization.
 * @return Pointer to the next token, or NULL when no more tokens remain.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/strtok_r.html
 */
char *strtok_r(char *str, const char *sep, char **state);

/**
 * @brief Return a string describing a signal number.
 * @ingroup posix_option_group_signals_ext
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
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_POSIX_STRING_H_ */
