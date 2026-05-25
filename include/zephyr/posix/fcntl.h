/*
 * SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX file control options (<fcntl.h>)
 *
 * Defines the file-control commands, file-descriptor flags, file-access
 * mode flags, and related types used with open(), creat(), and fcntl().
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/fcntl.h.html">
 *      POSIX.1-2017 &lt;fcntl.h&gt;</a>
 *
 * @defgroup posix_fcntl POSIX File Control
 * @ingroup posix_option_group_file_system
 * @{
 */

#ifndef ZEPHYR_POSIX_FCNTL_H_
#define ZEPHYR_POSIX_FCNTL_H_

#include <sys/stat.h>

#include <zephyr/sys/fdtable.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

/** @brief Duplicate a file descriptor to the lowest available descriptor >= arg. */
#define F_DUPFD ZVFS_F_DUPFD
#if (_POSIX_C_SOURCE >= 200809L) || defined(__DOXYGEN__)
/** @brief Duplicate a file descriptor with the close-on-exec flag set. */
#define F_DUPFD_CLOEXEC ZVFS_F_DUPFD_CLOEXEC
#endif
/** @brief Get file descriptor flags. */
#define F_GETFD  ZVFS_F_GETFD
/** @brief Set file descriptor flags. */
#define F_SETFD  ZVFS_F_SETFD
/** @brief Get file status flags and file access modes. */
#define F_GETFL  ZVFS_F_GETFL
/** @brief Set file status flags. */
#define F_SETFL  ZVFS_F_SETFL
/** @brief Get the first lock that blocks the lock described by the argument. */
#define F_GETLK  ZVFS_F_GETLK
/** @brief Set or clear a file segment lock (non-blocking). */
#define F_SETLK  ZVFS_F_SETLK
/** @brief Set or clear a file segment lock (blocking). */
#define F_SETLKW ZVFS_F_SETLKW
#if (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__)
/** @brief Get the process or process group ID that receives SIGIO/SIGURG signals. */
#define F_GETOWN ZVFS_F_GETOWN
/** @brief Set the process or process group ID that receives SIGIO/SIGURG signals. */
#define F_SETOWN ZVFS_F_SETOWN
#endif

/** @brief Close-on-exec flag for file descriptors. */
#define FD_CLOEXEC ZVFS_FD_CLOEXEC

/** @brief Lock type: shared read lock. */
#define F_RDLCK ZVFS_F_RDLCK
/** @brief Lock type: unlock (release a lock). */
#define F_UNLCK ZVFS_F_UNLCK
/** @brief Lock type: exclusive write lock. */
#define F_WRLCK ZVFS_F_WRLCK

/* SEEK_CUR, SEEK_END, SEEK_SET nominally defined in <stdio.h> */
/** @brief Set offset relative to current file position. */
#ifndef SEEK_CUR
#define SEEK_CUR ZVFS_SEEK_CUR
#endif

/** @brief Set offset relative to end of file. */
#ifndef SEEK_END
#define SEEK_END ZVFS_SEEK_END
#endif

/** @brief Set offset to absolute position. */
#ifndef SEEK_SET
#define SEEK_SET ZVFS_SEEK_SET
#endif

/** @brief Close file descriptor on exec (open() flag). */
#define O_CLOEXEC   ZVFS_O_CLOEXEC
/** @brief Create file if it does not exist (open() flag). */
#define O_CREAT     ZVFS_O_CREAT
/** @brief Fail if path does not name a directory (open() flag). */
#define O_DIRECTORY ZVFS_O_DIRECTORY
/** @brief Exclusive creation: fail if file already exists (open() flag, with O_CREAT). */
#define O_EXCL      ZVFS_O_EXCL
/** @brief Do not assign a controlling terminal (open() flag). */
#define O_NOCTTY    ZVFS_O_NOCTTY
/** @brief Do not follow symbolic links (open() flag). */
#define O_NOFOLLOW  ZVFS_O_NOFOLLOW
/** @brief Truncate file to zero length on open (open() flag). */
#define O_TRUNC     ZVFS_O_TRUNC
/** @brief Initialise terminal to conforming state (open() flag). */
#define O_TTY_INIT  ZVFS_O_TTY_INIT

/** @brief Write operations append to end of file. */
#define O_APPEND   ZVFS_O_APPEND
/** @brief Write I/O completes as defined for synchronised I/O data integrity. */
#define O_DSYNC    ZVFS_O_DSYNC
/** @brief Non-blocking I/O mode. */
#define O_NONBLOCK ZVFS_O_NONBLOCK
/** @brief Synchronised read I/O (same as O_SYNC for reads). */
#define O_RSYNC    ZVFS_O_RSYNC
/** @brief Write I/O completes as defined for synchronised I/O file integrity. */
#define O_SYNC     ZVFS_O_SYNC

/** @brief Open for execute only (no read, write, or search). */
#define O_EXEC   ZVFS_O_EXEC
/** @brief Open for reading only. */
#define O_RDONLY ZVFS_O_RDONLY
/** @brief Open for reading and writing. */
#define O_RDWR   ZVFS_O_RDWR
/** @brief Open directory for search only. */
#define O_SEARCH ZVFS_O_SEARCH
/** @brief Open for writing only. */
#define O_WRONLY ZVFS_O_WRONLY

/** @brief Mask to extract the file access mode from open flags. */
#define O_ACCMODE (ZVFS_O_RDONLY | ZVFS_O_RDWR | ZVFS_O_WRONLY)

/** @brief Equivalent to "." in the current directory (used with *at() functions). */
#define AT_FDCWD            ZVFS_AT_FDCWD
/** @brief Use effective IDs (faccessat flag). */
#define AT_EACCESS          ZVFS_AT_EACCESS
/** @brief Do not follow symbolic links (*at flag). */
#define AT_SYMLINK_NOFOLLOW ZVFS_AT_SYMLINK_NOFOLLOW
/** @brief Follow symbolic links (*at flag). */
#define AT_SYMLINK_FOLLOW   ZVFS_AT_SYMLINK_FOLLOW
/** @brief Remove directory (unlinkat flag). */
#define AT_REMOVEDIR        ZVFS_AT_REMOVEDIR

#if defined(_POSIX_ADVISORY_INFO) || defined(__DOXYGEN__)
/** @brief No advice (default access pattern). */
#define POSIX_FADV_NORMAL     ZVFS_POSIX_FADV_NORMAL
/** @brief Data will be accessed in random order. */
#define POSIX_FADV_RANDOM     ZVFS_POSIX_FADV_RANDOM
/** @brief Data will be accessed sequentially. */
#define POSIX_FADV_SEQUENTIAL ZVFS_POSIX_FADV_SEQUENTIAL
/** @brief Data will be needed in the near future. */
#define POSIX_FADV_WILLNEED   ZVFS_POSIX_FADV_WILLNEED
/** @brief Data will not be accessed in the near future. */
#define POSIX_FADV_DONTNEED   ZVFS_POSIX_FADV_DONTNEED
/** @brief Data will be accessed only once. */
#define POSIX_FADV_NOREUSE    ZVFS_POSIX_FADV_NOREUSE
#endif

#if !defined(_FLOCK_DECLARED) && !defined(__flock_defined)
/** @brief File segment locking structure used with F_GETLK, F_SETLK, and F_SETLKW. */
struct flock {
	short l_type;   /**< Lock type: F_RDLCK, F_WRLCK, or F_UNLCK. */
	short l_whence; /**< How to interpret l_start: SEEK_SET, SEEK_CUR, SEEK_END. */
	long l_start;   /**< Byte offset to start of locked region. */
	long l_len;     /**< Length of locked region in bytes (0 = to EOF). */
	pid_t l_pid;    /**< PID of the process holding the lock (F_GETLK output only). */
};
#define _FLOCK_DECLARED
#define __flock_defined
#endif

#if !defined(_MODE_T_DECLARED) && !defined(__mode_t_defined)
/** @brief File permission bits type. */
typedef int mode_t;
#define _MODE_T_DECLARED
#define __mode_t_defined
#endif

#if !defined(_OFF_T_DECLARED) && !defined(__off_t_defined)
/** @brief File offset type. */
typedef long off_t;
#define _OFF_T_DECLARED
#define __off_t_defined
#endif

#if !defined(_PID_T_DECLARED) && !defined(__pid_t_defined)
/** @brief Process ID type. */
typedef int pid_t;
#define _PID_T_DECLARED
#define __pid_t_defined
#endif

/**
 * @brief Create or truncate a file.
 *
 * Equivalent to @c open(path, O_WRONLY|O_CREAT|O_TRUNC, mode).
 *
 * @param path File path.
 * @param mode Permission bits applied if the file is created.
 * @return New file descriptor on success, or -1 with errno set on failure.
 */
int creat(const char *path, mode_t mode);

/**
 * @brief Perform file control operations on an open file descriptor.
 * @param fildes File descriptor.
 * @param cmd    Control command (F_GETFD, F_SETFD, F_GETFL, F_SETFL, F_GETLK, …).
 * @param ...    Optional argument (type depends on @p cmd).
 * @return Command-specific value on success, or -1 with errno set on failure.
 */
int fcntl(int fildes, int cmd, ...);

/**
 * @brief Open or create a file.
 * @param name  File path.
 * @param flags Access and creation flags (O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, …).
 * @param ...   If O_CREAT: mode_t mode.
 * @return New file descriptor on success, or -1 with errno set on failure.
 */
int open(const char *name, int flags, ...);

/**
 * @brief Open or create a file relative to a directory file descriptor.
 * @param fd    Directory file descriptor, or AT_FDCWD.
 * @param path  File path (relative to @p fd, or absolute).
 * @param oflag Access and creation flags.
 * @param ...   If O_CREAT: mode_t mode.
 * @return New file descriptor on success, or -1 with errno set on failure.
 */
int openat(int fd, const char *path, int oflag, ...);

#if defined(_POSIX_ADVISORY_INFO) || defined(__DOXYGEN__)
/**
 * @brief Declare an expected access pattern for a file region.
 * @param fd     File descriptor.
 * @param offset Start of the region.
 * @param len    Length of the region in bytes (0 = to EOF).
 * @param advice Access pattern hint (POSIX_FADV_*).
 * @return 0 on success, or a positive error number on failure.
 */
int posix_fadvise(int fd, off_t offset, off_t len, int advice);

/**
 * @brief Guarantee that disk space is allocated for a file region.
 * @param fd     File descriptor.
 * @param offset Start of the region.
 * @param len    Length of the region in bytes.
 * @return 0 on success, or a positive error number on failure.
 */
int posix_fallocate(int fd, off_t offset, off_t len);
#endif

/** @} */

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_POSIX_FCNTL_H_ */
