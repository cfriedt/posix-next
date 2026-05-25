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
 */

#ifndef ZEPHYR_POSIX_FCNTL_H_
#define ZEPHYR_POSIX_FCNTL_H_

#include <sys/stat.h>

#include <zephyr/sys/fdtable.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

/* --- fcntl() command codes (POSIX_FD_MGMT) -------------------------------- */

/** @brief Duplicate a file descriptor to the lowest available descriptor >= arg. @ingroup posix_option_group_fd_mgmt */
#define F_DUPFD ZVFS_F_DUPFD
#if (_POSIX_C_SOURCE >= 200809L) || defined(__DOXYGEN__)
/** @brief Duplicate a file descriptor with the close-on-exec flag set. @ingroup posix_option_group_fd_mgmt */
#define F_DUPFD_CLOEXEC ZVFS_F_DUPFD_CLOEXEC
#endif
/** @brief Get file descriptor flags. @ingroup posix_option_group_fd_mgmt */
#define F_GETFD  ZVFS_F_GETFD
/** @brief Set file descriptor flags. @ingroup posix_option_group_fd_mgmt */
#define F_SETFD  ZVFS_F_SETFD
/** @brief Get file status flags and file access modes. @ingroup posix_option_group_fd_mgmt */
#define F_GETFL  ZVFS_F_GETFL
/** @brief Set file status flags. @ingroup posix_option_group_fd_mgmt */
#define F_SETFL  ZVFS_F_SETFL
/** @brief Get the first lock that blocks the lock described by the argument. @ingroup posix_option_group_file_locking */
#define F_GETLK  ZVFS_F_GETLK
/** @brief Set or clear a file segment lock (non-blocking). @ingroup posix_option_group_file_locking */
#define F_SETLK  ZVFS_F_SETLK
/** @brief Set or clear a file segment lock (blocking). @ingroup posix_option_group_file_locking */
#define F_SETLKW ZVFS_F_SETLKW
#if (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__)
/** @brief Get the process or process group ID that receives SIGIO/SIGURG signals. @ingroup posix_option_group_fd_mgmt */
#define F_GETOWN ZVFS_F_GETOWN
/** @brief Set the process or process group ID that receives SIGIO/SIGURG signals. @ingroup posix_option_group_fd_mgmt */
#define F_SETOWN ZVFS_F_SETOWN
#endif

/** @brief Close-on-exec flag for file descriptors. @ingroup posix_option_group_fd_mgmt */
#define FD_CLOEXEC ZVFS_FD_CLOEXEC

/* --- File lock type codes (POSIX_FILE_LOCKING) ----------------------------- */

/** @brief Lock type: shared read lock. @ingroup posix_option_group_file_locking */
#define F_RDLCK ZVFS_F_RDLCK
/** @brief Lock type: unlock (release a lock). @ingroup posix_option_group_file_locking */
#define F_UNLCK ZVFS_F_UNLCK
/** @brief Lock type: exclusive write lock. @ingroup posix_option_group_file_locking */
#define F_WRLCK ZVFS_F_WRLCK

/* SEEK_CUR, SEEK_END, SEEK_SET nominally defined in <stdio.h> */
#ifndef SEEK_CUR
/** @brief Set offset relative to current file position. @ingroup posix_option_group_fd_mgmt */
#define SEEK_CUR ZVFS_SEEK_CUR
#endif

#ifndef SEEK_END
/** @brief Set offset relative to end of file. @ingroup posix_option_group_fd_mgmt */
#define SEEK_END ZVFS_SEEK_END
#endif

#ifndef SEEK_SET
/** @brief Set offset to absolute position. @ingroup posix_option_group_fd_mgmt */
#define SEEK_SET ZVFS_SEEK_SET
#endif

/* --- open() creation and status flags (POSIX_DEVICE_IO) ------------------- */

/** @brief Close file descriptor on exec (open() flag). @ingroup posix_option_group_device_io */
#define O_CLOEXEC   ZVFS_O_CLOEXEC
/** @brief Create file if it does not exist (open() flag). @ingroup posix_option_group_device_io */
#define O_CREAT     ZVFS_O_CREAT
/** @brief Fail if path does not name a directory (open() flag). @ingroup posix_option_group_device_io */
#define O_DIRECTORY ZVFS_O_DIRECTORY
/** @brief Exclusive creation: fail if file already exists (open() flag, with O_CREAT). @ingroup posix_option_group_device_io */
#define O_EXCL      ZVFS_O_EXCL
/** @brief Do not assign a controlling terminal (open() flag). @ingroup posix_option_group_device_io */
#define O_NOCTTY    ZVFS_O_NOCTTY
/** @brief Do not follow symbolic links (open() flag). @ingroup posix_option_group_device_io */
#define O_NOFOLLOW  ZVFS_O_NOFOLLOW
/** @brief Truncate file to zero length on open (open() flag). @ingroup posix_option_group_device_io */
#define O_TRUNC     ZVFS_O_TRUNC
/** @brief Initialise terminal to conforming state (open() flag). @ingroup posix_option_group_device_io */
#define O_TTY_INIT  ZVFS_O_TTY_INIT

/** @brief Write operations append to end of file. @ingroup posix_option_group_device_io */
#define O_APPEND   ZVFS_O_APPEND
/** @brief Write I/O completes as defined for synchronised I/O data integrity. @ingroup posix_option_group_device_io */
#define O_DSYNC    ZVFS_O_DSYNC
/** @brief Non-blocking I/O mode. @ingroup posix_option_group_device_io */
#define O_NONBLOCK ZVFS_O_NONBLOCK
/** @brief Synchronised read I/O (same as O_SYNC for reads). @ingroup posix_option_group_device_io */
#define O_RSYNC    ZVFS_O_RSYNC
/** @brief Write I/O completes as defined for synchronised I/O file integrity. @ingroup posix_option_group_device_io */
#define O_SYNC     ZVFS_O_SYNC

/** @brief Open for execute only (no read, write, or search). @ingroup posix_option_group_device_io */
#define O_EXEC   ZVFS_O_EXEC
/** @brief Open for reading only. @ingroup posix_option_group_device_io */
#define O_RDONLY ZVFS_O_RDONLY
/** @brief Open for reading and writing. @ingroup posix_option_group_device_io */
#define O_RDWR   ZVFS_O_RDWR
/** @brief Open directory for search only. @ingroup posix_option_group_device_io */
#define O_SEARCH ZVFS_O_SEARCH
/** @brief Open for writing only. @ingroup posix_option_group_device_io */
#define O_WRONLY ZVFS_O_WRONLY

/** @brief Mask to extract the file access mode from open flags. @ingroup posix_option_group_device_io */
#define O_ACCMODE (ZVFS_O_RDONLY | ZVFS_O_RDWR | ZVFS_O_WRONLY)

/* --- *at() directory-relative flags (POSIX_FILE_SYSTEM) ------------------- */

/** @brief Equivalent to "." in the current directory (used with *at() functions). @ingroup posix_option_group_file_system */
#define AT_FDCWD            ZVFS_AT_FDCWD
/** @brief Use effective IDs (faccessat flag). @ingroup posix_option_group_file_system */
#define AT_EACCESS          ZVFS_AT_EACCESS
/** @brief Do not follow symbolic links (*at flag). @ingroup posix_option_group_file_system */
#define AT_SYMLINK_NOFOLLOW ZVFS_AT_SYMLINK_NOFOLLOW
/** @brief Follow symbolic links (*at flag). @ingroup posix_option_group_file_system */
#define AT_SYMLINK_FOLLOW   ZVFS_AT_SYMLINK_FOLLOW
/** @brief Remove directory (unlinkat flag). @ingroup posix_option_group_file_system */
#define AT_REMOVEDIR        ZVFS_AT_REMOVEDIR

#if defined(_POSIX_ADVISORY_INFO) || defined(__DOXYGEN__)
/** @brief No advice (default access pattern). @ingroup posix_option_group_file_system */
#define POSIX_FADV_NORMAL     ZVFS_POSIX_FADV_NORMAL
/** @brief Data will be accessed in random order. @ingroup posix_option_group_file_system */
#define POSIX_FADV_RANDOM     ZVFS_POSIX_FADV_RANDOM
/** @brief Data will be accessed sequentially. @ingroup posix_option_group_file_system */
#define POSIX_FADV_SEQUENTIAL ZVFS_POSIX_FADV_SEQUENTIAL
/** @brief Data will be needed in the near future. @ingroup posix_option_group_file_system */
#define POSIX_FADV_WILLNEED   ZVFS_POSIX_FADV_WILLNEED
/** @brief Data will not be accessed in the near future. @ingroup posix_option_group_file_system */
#define POSIX_FADV_DONTNEED   ZVFS_POSIX_FADV_DONTNEED
/** @brief Data will be accessed only once. @ingroup posix_option_group_file_system */
#define POSIX_FADV_NOREUSE    ZVFS_POSIX_FADV_NOREUSE
#endif

#if !defined(_FLOCK_DECLARED) && !defined(__flock_defined)
/**
 * @brief File segment locking structure used with F_GETLK, F_SETLK, and F_SETLKW.
 * @ingroup posix_option_group_file_locking
 */
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
 * @ingroup posix_option_group_device_io
 *
 * Equivalent to @c open(path, O_WRONLY|O_CREAT|O_TRUNC, mode).
 *
 * @param path File path.
 * @param mode Permission bits applied if the file is created.
 * @return New file descriptor on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/creat.html
 */
int creat(const char *path, mode_t mode);

/**
 * @brief Perform file control operations on an open file descriptor.
 * @ingroup posix_option_group_fd_mgmt
 * @param fildes File descriptor.
 * @param cmd    Control command (F_GETFD, F_SETFD, F_GETFL, F_SETFL, F_GETLK, …).
 * @param ...    Optional argument (type depends on @p cmd).
 * @return Command-specific value on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fcntl.html
 */
int fcntl(int fildes, int cmd, ...);

/**
 * @brief Open or create a file.
 * @ingroup posix_option_group_device_io
 * @param name  File path.
 * @param flags Access and creation flags (O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, …).
 * @param ...   If O_CREAT: mode_t mode.
 * @return New file descriptor on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html
 */
int open(const char *name, int flags, ...);

/**
 * @brief Open or create a file relative to a directory file descriptor.
 * @ingroup posix_option_group_device_io
 * @param fd    Directory file descriptor, or AT_FDCWD.
 * @param path  File path (relative to @p fd, or absolute).
 * @param oflag Access and creation flags.
 * @param ...   If O_CREAT: mode_t mode.
 * @return New file descriptor on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/openat.html
 */
int openat(int fd, const char *path, int oflag, ...);

#if defined(_POSIX_ADVISORY_INFO) || defined(__DOXYGEN__)
/**
 * @brief Declare an expected access pattern for a file region.
 * @ingroup posix_option_group_file_system
 * @param fd     File descriptor.
 * @param offset Start of the region.
 * @param len    Length of the region in bytes (0 = to EOF).
 * @param advice Access pattern hint (POSIX_FADV_*).
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_fadvise.html
 */
int posix_fadvise(int fd, off_t offset, off_t len, int advice);

/**
 * @brief Guarantee that disk space is allocated for a file region.
 * @ingroup posix_option_group_file_system
 * @param fd     File descriptor.
 * @param offset Start of the region.
 * @param len    Length of the region in bytes.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_fallocate.html
 */
int posix_fallocate(int fd, off_t offset, off_t len);
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_POSIX_FCNTL_H_ */
