/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX pathname pattern-matching types (<glob.h>)
 *
 * The structure layout, flag values, and (negative) error values follow the
 * BSD <glob.h> that Newlib and Picolibc ship, so the implementation has one
 * ABI regardless of which header a consumer resolves.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/glob.h.html">
 *      POSIX.1-2017 &lt;glob.h&gt;</a>
 *
 */

#ifndef ZEPHYR_INCLUDE_POSIX_GLOB_H_
#define ZEPHYR_INCLUDE_POSIX_GLOB_H_

#ifdef __cplusplus
extern "C" {
#endif

struct dirent;
struct stat;

/**
 * @brief Pathname pattern-matching results.
 * @ingroup posix_option_group_file_system_glob
 */
typedef struct {
	/** Count of paths matched by pattern */
	int gl_pathc;
	/** Count of paths matching the most recent call */
	int gl_matchc;
	/** Slots to reserve at the beginning of gl_pathv */
	int gl_offs;
	/** Copy of the flags parameter to glob() */
	int gl_flags;
	/** Pointer to a list of matched pathnames */
	char **gl_pathv;
	/** Copy of the errfunc parameter to glob() */
	int (*gl_errfunc)(const char *epath, int eerrno);
	/** Alternate closedir(); unused */
	void (*gl_closedir)(void *dirp);
	/** Alternate readdir(); unused */
	struct dirent *(*gl_readdir)(void *dirp);
	/** Alternate opendir(); unused */
	void *(*gl_opendir)(const char *path);
	/** Alternate lstat(); unused */
	int (*gl_lstat)(const char *path, struct stat *buf);
	/** Alternate stat(); unused */
	int (*gl_stat)(const char *path, struct stat *buf);
} glob_t;

/**
 * @brief Flag: append generated pathnames to those from a previous call.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_APPEND
#define GLOB_APPEND 0x0001
/**
 * @brief Flag: make use of gl_offs.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_DOOFFS
#define GLOB_DOOFFS 0x0002
/**
 * @brief Flag: return on directory read error.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_ERR
#define GLOB_ERR 0x0004
/**
 * @brief Flag: mark each matched directory with a trailing slash.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_MARK
#define GLOB_MARK 0x0008
/**
 * @brief Flag: return the pattern itself when it matches nothing.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_NOCHECK
#define GLOB_NOCHECK 0x0010
/**
 * @brief Flag: do not sort the pathnames.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_NOSORT
#define GLOB_NOSORT 0x0020
/**
 * @brief Flag: disable backslash escaping.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_NOESCAPE
#define GLOB_NOESCAPE 0x2000

/**
 * @brief Return value of glob() indicating an attempt to allocate memory failed.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_NOSPACE
#define GLOB_NOSPACE (-1)
/**
 * @brief Return value of glob() indicating the scan was stopped by an error.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_ABORTED
#define GLOB_ABORTED (-2)
/**
 * @brief Return value of glob() indicating the pattern matched no existing pathname.
 * @ingroup posix_option_group_file_system_glob
 */
#undef GLOB_NOMATCH
#define GLOB_NOMATCH (-3)

/**
 * @brief Generate pathnames matching a pattern.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/glob.html
 */
int glob(const char *__restrict pattern, int flags,
	 int (*errfunc)(const char *epath, int eerrno), glob_t *__restrict pglob);

/**
 * @brief Free memory associated with a glob_t.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/globfree.html
 */
void globfree(glob_t *pglob);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_GLOB_H_ */
