/*
 * SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_POSIX_FCNTL_H_
#define ZEPHYR_POSIX_FCNTL_H_

#include <sys/stat.h>

#include <zephyr/sys/fdtable.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#define F_DUPFD ZVFS_F_DUPFD
#if (_POSIX_C_SOURCE >= 200809L) || defined(__DOXYGEN__)
#define F_DUPFD_CLOEXEC ZVFS_F_DUPFD_CLOEXEC
#endif
#define F_GETFD  ZVFS_F_GETFD
#define F_SETFD  ZVFS_F_SETFD
#define F_GETFL  ZVFS_F_GETFL
#define F_SETFL  ZVFS_F_SETFL
#define F_GETLK  ZVFS_F_GETLK
#define F_SETLK  ZVFS_F_SETLK
#define F_SETLKW ZVFS_F_SETLKW
#if (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__)
#define F_GETOWN ZVFS_F_GETOWN
#define F_SETOWN ZVFS_F_SETOWN
#endif

#define FD_CLOEXEC ZVFS_FD_CLOEXEC

#define F_RDLCK ZVFS_F_RDLCK
#define F_UNLCK ZVFS_F_UNLCK
#define F_WRLCK ZVFS_F_WRLCK

/* SEEK_CUR, SEEK_END, SEEK_SET nominally defined in <stdio.h> */
#ifndef SEEK_CUR
#define SEEK_CUR ZVFS_SEEK_CUR
#endif

#ifndef SEEK_END
#define SEEK_END ZVFS_SEEK_END
#endif

#ifndef SEEK_SET
#define SEEK_SET ZVFS_SEEK_SET
#endif

#define O_CLOEXEC   ZVFS_O_CLOEXEC
#define O_CREAT     ZVFS_O_CREAT
#define O_DIRECTORY ZVFS_O_DIRECTORY
#define O_EXCL      ZVFS_O_EXCL
#define O_NOCTTY    ZVFS_O_NOCTTY
#define O_NOFOLLOW  ZVFS_O_NOFOLLOW
#define O_TRUNC     ZVFS_O_TRUNC
#define O_TTY_INIT  ZVFS_O_TTY_INIT

#define O_APPEND   ZVFS_O_APPEND
#define O_DSYNC    ZVFS_O_DSYNC
#define O_NONBLOCK ZVFS_O_NONBLOCK
#define O_RSYNC    ZVFS_O_RSYNC
#define O_SYNC     ZVFS_O_SYNC

#define O_EXEC   ZVFS_O_EXEC
#define O_RDONLY ZVFS_O_RDONLY
#define O_RDWR   ZVFS_O_RDWR
#define O_SEARCH ZVFS_O_SEARCH
#define O_WRONLY ZVFS_O_WRONLY

#define O_ACCMODE (ZVFS_O_RDONLY | ZVFS_O_RDWR | ZVFS_O_WRONLY)

#define AT_FDCWD            ZVFS_AT_FDCWD
#define AT_EACCESS          ZVFS_AT_EACCESS
#define AT_SYMLINK_NOFOLLOW ZVFS_AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_FOLLOW   ZVFS_AT_SYMLINK_FOLLOW
#define AT_REMOVEDIR        ZVFS_AT_REMOVEDIR

#if defined(_POSIX_ADVISORY_INFO) || defined(__DOXYGEN__)
#define POSIX_FADV_NORMAL     ZVFS_POSIX_FADV_NORMAL
#define POSIX_FADV_RANDOM     ZVFS_POSIX_FADV_RANDOM
#define POSIX_FADV_SEQUENTIAL ZVFS_POSIX_FADV_SEQUENTIAL
#define POSIX_FADV_WILLNEED   ZVFS_POSIX_FADV_WILLNEED
#define POSIX_FADV_DONTNEED   ZVFS_POSIX_FADV_DONTNEED
#define POSIX_FADV_NOREUSE    ZVFS_POSIX_FADV_NOREUSE
#endif

#if !defined(_FLOCK_DECLARED) && !defined(__flock_defined)
struct flock {
	short l_type;
	short l_whence;
	long l_start;
	long l_len;
	pid_t l_pid;
};
#define _FLOCK_DECLARED
#define __flock_defined
#endif

#if !defined(_MODE_T_DECLARED) && !defined(__mode_t_defined)
typedef int mode_t;
#define _MODE_T_DECLARED
#define __mode_t_defined
#endif

#if !defined(_OFF_T_DECLARED) && !defined(__off_t_defined)
typedef long off_t;
#define _OFF_T_DECLARED
#define __off_t_defined
#endif

#if !defined(_PID_T_DECLARED) && !defined(__pid_t_defined)
/* TODO: it would be nice to convert this to a long */
typedef int pid_t;
#define _PID_T_DECLARED
#define __pid_t_defined
#endif

int creat(const char *path, mode_t mode);
int fcntl(int fildes, int cmd, ...);
int open(const char *name, int flags, ...);
int openat(int fd, const char *path, int oflag, ...);
#if defined(_POSIX_ADVISORY_INFO) || defined(__DOXYGEN__)
int posix_fadvise(int fd, off_t offset, off_t len, int advice);
int posix_fallocate(int fd, off_t offset, off_t len);
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_POSIX_FCNTL_H_ */
