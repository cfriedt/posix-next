/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX pathname configuration constants (<sys/pathconf.h>)
 *
 * Defines the @c _PC_* name constants used as the second argument to
 * pathconf() and fpathconf() to query per-file configuration limits.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/functions/pathconf.html">
 *      POSIX.1-2017 pathconf()</a>
 *
 * @ingroup posix_option_group_file_system
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_PATHCONF_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_PATHCONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Toolchains (e.g. picolibc) may already define _PC_* as numeric macros via
 * <unistd.h>. Do not reintroduce them as enum enumerators in that case.
 */
#ifndef _PC_LINK_MAX
/** @brief pathconf() / fpathconf() name constants. */
enum {
	_PC_2_SYMLINKS,           /**< Pathname creates a symbolic link. */
	_PC_ALLOC_SIZE_MIN,       /**< Minimum storage actually allocated for data. */
	_PC_ASYNC_IO,             /**< Asynchronous I/O may be performed. */
	_PC_CHOWN_RESTRICTED,     /**< Use of chown() is restricted. */
	_PC_FILESIZEBITS,         /**< Minimum bits to represent maximum file size. */
	_PC_LINK_MAX,             /**< Maximum number of links to the file. */
	_PC_MAX_CANON,            /**< Maximum bytes in a terminal canonical input line. */
	_PC_MAX_INPUT,            /**< Minimum bytes for which space is available in a terminal input queue. */
	_PC_NAME_MAX,             /**< Maximum number of bytes in a file name. */
	_PC_NO_TRUNC,             /**< Filenames longer than NAME_MAX generate an error. */
	_PC_PATH_MAX,             /**< Maximum number of bytes in a pathname. */
	_PC_PIPE_BUF,             /**< Maximum number of bytes written atomically to a pipe. */
	_PC_PRIO_IO,              /**< Prioritized I/O is supported. */
	_PC_REC_INCR_XFER_SIZE,   /**< Recommended increment for efficient I/O transfers. */
	_PC_REC_MAX_XFER_SIZE,    /**< Maximum recommended transfer size for efficient I/O. */
	_PC_REC_MIN_XFER_SIZE,    /**< Minimum recommended transfer size for efficient I/O. */
	_PC_REC_XFER_ALIGN,       /**< Recommended buffer alignment for efficient I/O transfers. */
	_PC_SYMLINK_MAX,          /**< Maximum number of bytes in a symbolic link. */
	_PC_SYNC_IO,              /**< Synchronized I/O may be performed. */
	_PC_TIMESTAMP_RESOLUTION, /**< Resolution of file timestamps in nanoseconds. */
	_PC_VDISABLE,             /**< Terminal special characters can be disabled. */
};
#endif /* !_PC_LINK_MAX */


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_PATHCONF_H_ */
