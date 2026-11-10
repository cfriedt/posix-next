/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX regular expression types (<regex.h>)
 *
 * The structure layout, flag values, and error values follow the BSD
 * <regex.h> that Newlib and Picolibc ship (Henry Spencer's regex), so the
 * implementation has one ABI regardless of which header a consumer resolves.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/regex.h.html">
 *      POSIX.1-2017 &lt;regex.h&gt;</a>
 *
 */

#ifndef ZEPHYR_INCLUDE_POSIX_REGEX_H_
#define ZEPHYR_INCLUDE_POSIX_REGEX_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Signed offset into a matched string */
typedef ptrdiff_t regoff_t;

/** A compiled regular expression */
typedef struct {
	unsigned int re_magic;  /**< implementation detail */
	size_t re_nsub;         /**< number of parenthesized subexpressions */
	const char *re_endp;    /**< end pointer for #REG_PEND */
	struct re_guts *re_g;   /**< implementation detail */
} regex_t;

/** One (sub)expression match reported by regexec() */
typedef struct {
	regoff_t rm_so; /**< byte offset of the start of the match */
	regoff_t rm_eo; /**< byte offset of the first byte after the match */
} regmatch_t;

/**
 * @name regcomp() flags
 * @{
 */
#define REG_BASIC    0000 /**< Basic Regular Expressions (the default) */
#define REG_EXTENDED 0001 /**< Extended Regular Expressions */
#define REG_ICASE    0002 /**< ignore case in match */
#define REG_NOSUB    0004 /**< report only success or failure in regexec() */
#define REG_NEWLINE  0010 /**< change the handling of newline */
#define REG_NOSPEC   0020 /**< no special characters (literal string match) */
#define REG_PEND     0040 /**< pattern ends at regex_t::re_endp */
#define REG_DUMP     0200 /**< reserved for debugging */
/** @} */

/**
 * @name regcomp() and regexec() error values
 * @{
 */
#define REG_NOMATCH  1    /**< regexec() failed to match */
#define REG_BADPAT   2    /**< invalid regular expression */
#define REG_ECOLLATE 3    /**< invalid collating element referenced */
#define REG_ECTYPE   4    /**< invalid character class type referenced */
#define REG_EESCAPE  5    /**< trailing backslash in pattern */
#define REG_ESUBREG  6    /**< number in \\digit invalid or in error */
#define REG_EBRACK   7    /**< "[]" imbalance */
#define REG_EPAREN   8    /**< "\\(\\)" or "()" imbalance */
#define REG_EBRACE   9    /**< "\\{\\}" imbalance */
#define REG_BADBR    10   /**< content of "\\{\\}" invalid */
#define REG_ERANGE   11   /**< invalid endpoint in range expression */
#define REG_ESPACE   12   /**< out of memory */
#define REG_BADRPT   13   /**< '?', '*', or '+' not preceded by valid RE */
#define REG_EMPTY    14   /**< empty (sub)expression */
#define REG_ASSERT   15   /**< internal assertion ("can't happen") */
#define REG_INVARG   16   /**< invalid argument */
#define REG_ATOI     255  /**< convert name to number (debugging) */
#define REG_ITOA     0400 /**< convert number to name (debugging) */
/** @} */

/**
 * @name regexec() flags
 * @{
 */
#define REG_NOTBOL   00001 /**< first character is not the beginning of line */
#define REG_NOTEOL   00002 /**< last character is not the end of line */
#define REG_STARTEND 00004 /**< string bounded by pmatch[0] on entry */
#define REG_TRACE    00400 /**< reserved for debugging */
#define REG_LARGE    01000 /**< reserved for debugging */
#define REG_BACKR    02000 /**< reserved for debugging */
/** @} */

/**
 * @brief Compile a regular expression
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/functions/regcomp.html">
 *      POSIX.1-2017 regcomp()</a>
 */
int regcomp(regex_t *__restrict preg, const char *__restrict pattern, int cflags);

/**
 * @brief Translate a regcomp() or regexec() error value into a message
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/functions/regerror.html">
 *      POSIX.1-2017 regerror()</a>
 */
size_t regerror(int errcode, const regex_t *__restrict preg, char *__restrict errbuf,
		size_t errbuf_size);

/**
 * @brief Match a compiled regular expression against a string
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/functions/regexec.html">
 *      POSIX.1-2017 regexec()</a>
 */
int regexec(const regex_t *__restrict preg, const char *__restrict string, size_t nmatch,
	    regmatch_t *__restrict pmatch, int eflags);

/**
 * @brief Release the storage held by a compiled regular expression
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/functions/regfree.html">
 *      POSIX.1-2017 regfree()</a>
 */
void regfree(regex_t *preg);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_REGEX_H_ */
