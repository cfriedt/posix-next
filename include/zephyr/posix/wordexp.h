/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX word expansion types (<wordexp.h>)
 *
 * The structure layout and the flag and error values follow the <wordexp.h>
 * that Newlib and Picolibc ship, so one implementation serves whichever
 * header a consumer resolves.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/wordexp.h.html">
 *      POSIX.1-2017 &lt;wordexp.h&gt;</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_WORDEXP_H_
#define ZEPHYR_INCLUDE_POSIX_WORDEXP_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Word expansion results.
 * @ingroup posix_option_group_shell_func
 */
typedef struct {
	/** Count of words matched by words */
	size_t we_wordc;
	/** Pointer to list of expanded words */
	char **we_wordv;
	/** Slots to reserve at the beginning of we_wordv */
	size_t we_offs;
} wordexp_t;

/** @brief Flag: use we_offs. @ingroup posix_option_group_shell_func */
#define WRDE_DOOFFS  0x0001
/** @brief Flag: append to the words of a previous call. @ingroup posix_option_group_shell_func */
#define WRDE_APPEND  0x0002
/** @brief Flag: fail on command substitution. @ingroup posix_option_group_shell_func */
#define WRDE_NOCMD   0x0004
/**
 * @brief Flag: pwordexp came from a previous successful call.
 * @ingroup posix_option_group_shell_func
 */
#define WRDE_REUSE   0x0008
/**
 * @brief Flag: do not redirect the shell's error messages.
 * @ingroup posix_option_group_shell_func
 */
#define WRDE_SHOWERR 0x0010
/** @brief Flag: an undefined shell variable is an error. @ingroup posix_option_group_shell_func */
#define WRDE_UNDEF   0x0020

/** @brief Result: success. @ingroup posix_option_group_shell_func */
#define WRDE_SUCCESS 0
/** @brief Result: an attempt to allocate memory failed. @ingroup posix_option_group_shell_func */
#define WRDE_NOSPACE 1
/** @brief Result: an unquoted character is invalid here. @ingroup posix_option_group_shell_func */
#define WRDE_BADCHAR 2
/**
 * @brief Result: an undefined variable was expanded under WRDE_UNDEF.
 * @ingroup posix_option_group_shell_func
 */
#define WRDE_BADVAL  3
/**
 * @brief Result: command substitution was requested under WRDE_NOCMD.
 * @ingroup posix_option_group_shell_func
 */
#define WRDE_CMDSUB  4
/** @brief Result: a shell syntax error. @ingroup posix_option_group_shell_func */
#define WRDE_SYNTAX  5
/** @brief Result: word expansion is not supported. @ingroup posix_option_group_shell_func */
#define WRDE_NOSYS   6

/**
 * @brief Perform word expansions.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/wordexp.html
 */
int wordexp(const char *__restrict words, wordexp_t *__restrict pwordexp, int flags);

/**
 * @brief Free the words of a wordexp_t.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/wordfree.html
 */
void wordfree(wordexp_t *pwordexp);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_WORDEXP_H_ */
