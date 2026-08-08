/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX file system information (<sys/statvfs.h>)
 *
 * Defines the statvfs structure and the statvfs() and fstatvfs() functions
 * used to retrieve information about a mounted file system.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_statvfs.h.html">
 *      POSIX.1-2017 &lt;sys/statvfs.h&gt;</a>
 *
 * @ingroup posix_option_group_file_system
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_STATVFS_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_STATVFS_H_

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

/* fsblkcnt_t and fsfilcnt_t are owned by <sys/types.h>; re-declared here as a
 * shared symbol. newlib and picolibc declare both under _FSBLKCNT_T_DECLARED.
 */
#if !defined(_FSBLKCNT_T_DECLARED) && !defined(__fsblkcnt_t_defined)
typedef unsigned long fsblkcnt_t;
typedef unsigned long fsfilcnt_t;
#define _FSBLKCNT_T_DECLARED
#define _FSFILCNT_T_DECLARED
#define __fsblkcnt_t_defined
#define __fsfilcnt_t_defined
#endif

#if !defined(_STATVFS_DECLARED) && !defined(__statvfs_defined)
/**
 * @brief File system information returned by statvfs() and fstatvfs().
 * @ingroup posix_option_group_file_system
 */
struct statvfs {
	unsigned long f_bsize;  /**< File system block size. */
	unsigned long f_frsize; /**< Fundamental file system block size. */
	fsblkcnt_t f_blocks;    /**< Total blocks on file system, in units of f_frsize. */
	fsblkcnt_t f_bfree;     /**< Total number of free blocks. */
	fsblkcnt_t f_bavail;    /**< Free blocks available to non-privileged processes. */
	fsfilcnt_t f_files;     /**< Total number of file serial numbers. */
	fsfilcnt_t f_ffree;     /**< Total number of free file serial numbers. */
	fsfilcnt_t f_favail;    /**< File serial numbers available to non-privileged processes. */
	unsigned long f_fsid;   /**< File system ID. */
	unsigned long f_flag;   /**< Bit mask of ST_* values. */
	unsigned long f_namemax; /**< Maximum filename length. */
};
#define _STATVFS_DECLARED
#define __statvfs_defined
#endif

/** @brief Read-only file system. @ingroup posix_option_group_file_system */
#define ST_RDONLY 0x1
/** @brief Setuid and setgid bits are ignored. @ingroup posix_option_group_file_system */
#define ST_NOSUID 0x2

/**
 * @brief Get file system information for the file system containing an open file.
 * @ingroup posix_option_group_file_system
 * @param fildes File descriptor of an open file.
 * @param buf Destination for the file system information.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fstatvfs.html
 */
int fstatvfs(int fildes, struct statvfs *buf);

/**
 * @brief Get file system information by path.
 * @ingroup posix_option_group_file_system
 * @param path Path of any file within the mounted file system.
 * @param buf Destination for the file system information.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/statvfs.html
 */
int statvfs(const char *ZRESTRICT path, struct statvfs *ZRESTRICT buf);

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_STATVFS_H_ */
