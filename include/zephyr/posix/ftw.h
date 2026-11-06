/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief File tree traversal (<ftw.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/ftw.h.html">
 *      POSIX.1-2017 &lt;ftw.h&gt;</a>
 *
 * @ingroup posix_option_group_xsi_file_system
 */

#ifndef ZEPHYR_INCLUDE_POSIX_FTW_H_
#define ZEPHYR_INCLUDE_POSIX_FTW_H_

#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)

/**
 * @brief Position of a reported object in the walk.
 * @ingroup posix_option_group_xsi_file_system
 */
struct FTW {
	int base;  /**< Offset of the object's name within the reported path. */
	int level; /**< Depth of the object below the starting path. */
};

/**
 * @brief Non-directory file.
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_F 0
/**
 * @brief Directory.
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_D 1
/**
 * @brief Directory that cannot be read.
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_DNR 2
/**
 * @brief Directory, reported after its contents (FTW_DEPTH).
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_DP 3
/**
 * @brief Object whose status cannot be obtained.
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_NS 4
/**
 * @brief Symbolic link (FTW_PHYS).
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_SL 5
/**
 * @brief Symbolic link to an object that does not exist.
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_SLN 6

/**
 * @brief Report symbolic links instead of following them.
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_PHYS 1
/**
 * @brief Stay within the starting path's file system.
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_MOUNT 2
/**
 * @brief Report a directory after its contents.
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_DEPTH 4
/**
 * @brief Run the callback in the directory holding the reported object.
 * @ingroup posix_option_group_xsi_file_system
 */
#define FTW_CHDIR 8

/**
 * @brief Walk a file tree (obsolescent).
 * @ingroup posix_option_group_xsi_file_system
 * @param path Starting path.
 * @param fn Callback for every object; a nonzero return ends the walk.
 * @param ndirs Maximum number of directory descriptors the walk holds open.
 * @return 0, the callback's nonzero return, or -1 with errno set.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/ftw.html
 */
int ftw(const char *path, int (*fn)(const char *path, const struct stat *st, int flag),
	int ndirs);

/**
 * @brief Walk a file tree.
 * @ingroup posix_option_group_xsi_file_system
 * @param path Starting path.
 * @param fn Callback for every object; a nonzero return ends the walk.
 * @param fd_limit Maximum number of directory descriptors the walk holds open.
 * @param flags Any of FTW_PHYS, FTW_MOUNT, FTW_DEPTH and FTW_CHDIR.
 * @return 0, the callback's nonzero return, or -1 with errno set.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/nftw.html
 */
int nftw(const char *path,
	 int (*fn)(const char *path, const struct stat *st, int flag, struct FTW *ftwbuf),
	 int fd_limit, int flags);

#endif /* _XOPEN_SOURCE || __DOXYGEN__ */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_FTW_H_ */
