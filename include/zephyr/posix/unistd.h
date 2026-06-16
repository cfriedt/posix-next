/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX symbolic constants and miscellaneous functions (<unistd.h>)
 *
 * Provides symbolic constants for file access modes, seek positions, and
 * option groups, together with the full set of POSIX miscellaneous functions
 * (process control, file I/O, user/group IDs, and process environment).
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/unistd.h.html">
 *      POSIX.1-2017 &lt;unistd.h&gt;</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_UNISTD_H_
#define ZEPHYR_INCLUDE_POSIX_UNISTD_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include "posix_features.h"

#include <stddef.h>
#include <stdint.h>

#include <zephyr/posix/sys/confstr.h>
#include <zephyr/posix/sys/pathconf.h>
#include <zephyr/posix/sys/sysconf.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version test macros _POSIX_VERSION, _XOPEN_VERSION, etc, defined in posix_features.h */
/* Constants for Options and Option Groups defined in posix_features.h */

/* Execution-Time Symbolic Constants defined in posix_features.h */
/* TODO: define _POSIX_ASYNC_IO, _POSIX_PRIO_IO, _POSIX_SYNC_IO */
/* TODO: define _POSIX_TIMESTAMP_RESOLUTION, _POSIX2_SYMLINKS */

/* _POSIX2_VERSION defined by posix_features.h */
/* _XOPEN_VERSION defined by posix_features.h */

/* NULL must be defined by the libc stddef.h */

/**
 * @brief Test whether the file exists.
 * @ingroup posix_option_group_file_system
 */
#define F_OK 0
/**
 * @brief Test whether the file may be read.
 * @ingroup posix_option_group_file_system
 */
#define R_OK 4
/**
 * @brief Test whether the file may be written.
 * @ingroup posix_option_group_file_system
 */
#define W_OK 2
/**
 * @brief Test whether the file may be executed.
 * @ingroup posix_option_group_file_system
 */
#define X_OK 1

/* confstr() constants in zephyr/posix/sys/confstr.h */

/* SEEK_CUR, SEEK_END, SEEK_SET nominally defined in <stdio.h> */
#ifndef SEEK_CUR
/** @brief Seek relative to the current file offset. @ingroup posix_option_group_fd_mgmt */
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
/** @brief Seek relative to the end of the file. @ingroup posix_option_group_fd_mgmt */
#define SEEK_END 2
#endif

#ifndef SEEK_SET
/** @brief Seek relative to the start of the file. @ingroup posix_option_group_fd_mgmt */
#define SEEK_SET 0
#endif

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Lock a region of a file (lockf command). @ingroup posix_option_group_xsi_file_system */
#define F_LOCK  1
/** @brief Test whether a file region is locked (lockf command). @ingroup posix_option_group_xsi_file_system */
#define F_TEST  3
/** @brief Lock a region, failing immediately if blocked (lockf command). @ingroup posix_option_group_xsi_file_system */
#define F_TLOCK 2
/** @brief Unlock a region (lockf command). @ingroup posix_option_group_xsi_file_system */
#define F_ULOCK 0
#endif

/* pathconf() constants in zephyr/posix/sys/pathconf.h  */
/* sysconf() constants in zephyr/posix/sys/sysconf.h  */

/**
 * @brief File descriptor for standard error.
 * @ingroup posix_option_group_device_io
 */
#define STDERR_FILENO 2
/**
 * @brief File descriptor for standard input.
 * @ingroup posix_option_group_device_io
 */
#define STDIN_FILENO  0
/**
 * @brief File descriptor for standard output.
 * @ingroup posix_option_group_device_io
 */
#define STDOUT_FILENO 1

/**
 * @brief Value used to disable a special character in a terminal.
 * @ingroup posix_option_group_device_specific
 */
#define _POSIX_VDISABLE ('\0')

#if !defined(_GID_T_DECLARED) && !defined(__gid_t_defined)
typedef unsigned short gid_t;
#define _GID_T_DECLARED
#define __gid_t_defined
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

/* size_t must be defined by the libc stddef.h */

#if !defined(_SSIZE_T_DECLARED) && !defined(__ssize_t_defined)
#ifndef __SIZE_TYPE__
#define __SIZE_TYPE__ unsigned long
#endif
#define unsigned signed /* parasoft-suppress MISRAC2012-RULE_20_4-a MISRAC2012-RULE_20_4-b */
typedef __SIZE_TYPE__ ssize_t;
#undef unsigned
#define _SSIZE_T_DECLARED
#define __ssize_t_defined
#endif

#if !defined(_UID_T_DECLARED) && !defined(__uid_t_defined)
typedef unsigned short uid_t;
#define _UID_T_DECLARED
#define __uid_t_defined
#endif

#if (_POSIX_C_SOURCE < 200809L) || defined(__DOXYGEN__)
#if !defined(_USECONDS_T_DECLARED) && !defined(__useconds_t_defined)
typedef unsigned long useconds_t;
#define _USECONDS_T_DECLARED
#define __useconds_t_defined
#endif
#endif

/* intptr_t is optionally defined by the libc stdint.h and we assume it is defined there */

/**
 * @brief Determine accessibility of a file.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/access.html
 */
int access(const char *path, int amode);
/**
 * @brief Schedule an alarm signal (SIGALRM) after a given number of seconds.
 * @ingroup posix_option_group_signals
 */
unsigned int alarm(unsigned int seconds);
/**
 * @brief Change the current working directory.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/chdir.html
 */
int chdir(const char *path);
/**
 * @brief Change the owner and group of a file.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/chown.html
 */
int chown(const char *path, uid_t owner, gid_t group);
/**
 * @brief Close a file descriptor.
 * @ingroup posix_option_group_device_io
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/close.html
 */
int close(int fildes);
/**
 * @brief Determine the value of a configurable system variable by name string.
 * @ingroup posix_option_group_single_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/confstr.html
 */
size_t confstr(int name, char *buf, size_t len);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Encrypt a password string (XSI extension, not recommended for new code).
 * @ingroup posix_option_group_xsi_single_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/crypt.html
 */
char *crypt(const char *key, const char *salt);
#endif
/**
 * @brief Duplicate an open file descriptor to the lowest available descriptor.
 * @ingroup posix_option_group_fd_mgmt
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/dup.html
 */
int dup(int fildes);
/**
 * @brief Duplicate an open file descriptor to a specific descriptor number.
 * @ingroup posix_option_group_fd_mgmt
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/dup2.html
 */
int dup2(int fildes, int fildes2);
/**
 * @brief Terminate the calling process without calling atexit() handlers.
 * @ingroup posix_option_group_multi_process
 */
FUNC_NORETURN void _exit(int status);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Encrypt or decrypt a 64-bit block using DES (XSI extension).
 * @ingroup posix_option_group_xsi_single_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/encrypt.html
 */
void encrypt(char block[64], int edflag);
#endif
/**
 * @brief Replace the process image with a new program (argument list form).
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/execl.html
 */
int execl(const char *path, const char *arg0, ...);
/**
 * @brief Replace process image with a new program plus environment (argument list form).
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/execle.html
 */
int execle(const char *path, const char *arg0, ...);
/**
 * @brief Replace process image using PATH search (argument list form).
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/execlp.html
 */
int execlp(const char *file, const char *arg0, ...);
/**
 * @brief Replace process image with a new program (vector form).
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/execv.html
 */
int execv(const char *path, char *const argv[]);
/**
 * @brief Replace process image with a new program plus environment (vector form).
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/execve.html
 */
int execve(const char *path, char *const argv[], char *const envp[]);
/**
 * @brief Replace process image using PATH search (vector form).
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/execvp.html
 */
int execvp(const char *file, char *const argv[]);
/**
 * @brief Determine accessibility of a file relative to a directory descriptor.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/faccessat.html
 */
int faccessat(int fd, const char *path, int amode, int flag);
/**
 * @brief Change the current working directory to the directory named by a file descriptor.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchdir.html
 */
int fchdir(int fildes);
/**
 * @brief Change the owner and group of an open file.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchown.html
 */
int fchown(int fildes, uid_t owner, gid_t group);
/**
 * @brief Change owner and group of a file relative to a directory descriptor.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchownat.html
 */
int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flag);
#if defined(_POSIX_SYNCHRONIZED_IO) || defined(__DOXYGEN__)
/**
 * @brief Synchronise the data of an open file to storage (without metadata).
 * @ingroup posix_option_synchronized_io
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fdatasync.html
 */
int fdatasync(int fildes);
#endif
/**
 * @brief Replace process image using an open file descriptor (vector form).
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fexecve.html
 */
int fexecve(int fd, char *const argv[], char *const envp[]);
/**
 * @brief Create a child process.
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fork.html
 */
pid_t fork(void);
/**
 * @brief Determine the value of a configurable limit for an open file.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fpathconf.html
 */
long fpathconf(int fildes, int name);
#if defined(_POSIX_FSYNC) || defined(__DOXYGEN__)
/**
 * @brief Synchronise an open file's data and metadata to storage.
 * @ingroup posix_option_fsync
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fsync.html
 */
int fsync(int fd);
#endif
/**
 * @brief Truncate an open file to a specified length.
 * @ingroup posix_option_group_fd_mgmt
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/ftruncate.html
 */
int ftruncate(int fildes, off_t length);
/**
 * @brief Get the pathname of the current working directory.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getcwd.html
 */
char *getcwd(char *buf, size_t size);
/**
 * @brief Get the effective group ID of the calling process.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getegid.html
 */
gid_t getegid(void);
#if (_POSIX_C_SOURCE >= 202405L) || defined(__DOXYGEN__)
/**
 * @brief Fill a buffer with cryptographically secure random bytes.
 * @ingroup posix_option_group_c_lib_ext
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getentropy.html
 */
int getentropy(void *buffer, size_t length);
#endif
/**
 * @brief Get the effective user ID of the calling process.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/geteuid.html
 */
uid_t geteuid(void);
/**
 * @brief Get the real group ID of the calling process.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getgid.html
 */
gid_t getgid(void);
/**
 * @brief Get the supplementary group IDs of the calling process.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getgroups.html
 */
int getgroups(int gidsetsize, gid_t grouplist[]);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Get a unique identifier for the host system (XSI extension).
 * @ingroup posix_option_group_xsi_single_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/gethostid.html
 */
long gethostid(void);
#endif
/**
 * @brief Get the name of the current host.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/gethostname.html
 */
int gethostname(char *name, size_t namelen);
/**
 * @brief Get the login name of the user.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getlogin.html
 */
char *getlogin(void);
/**
 * @brief Get the login name of the user into a caller-supplied buffer.
 * @ingroup posix_option_group_user_groups_r
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getlogin_r.html
 */
int getlogin_r(char *name, size_t namesize);
/**
 * @brief Parse command-line options.
 * @ingroup posix_option_group_c_lib_ext
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getopt.html
 */
int getopt(int argc, char *const argv[], const char *optstring);
/**
 * @brief Get the process group ID of a process.
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpgid.html
 */
pid_t getpgid(pid_t pid);
/**
 * @brief Get the process group ID of the calling process.
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpgrp.html
 */
pid_t getpgrp(void);
/**
 * @brief Get the process ID of the calling process.
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpid.html
 */
pid_t getpid(void);
/**
 * @brief Get the process ID of the parent of the calling process.
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getppid.html
 */
pid_t getppid(void);
/**
 * @brief Get the session ID of a process.
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getsid.html
 */
pid_t getsid(pid_t pid);
/**
 * @brief Get the real user ID of the calling process.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getuid.html
 */
uid_t getuid(void);
/**
 * @brief Determine whether a file descriptor refers to a terminal.
 * @ingroup posix_option_group_device_specific
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/isatty.html
 */
int isatty(int fildes);
/**
 * @brief Change the owner and group of a symbolic link.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/lchown.html
 */
int lchown(const char *path, uid_t owner, gid_t group);
/**
 * @brief Create a hard link.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/link.html
 */
int link(const char *path1, const char *path2);
/**
 * @brief Create a hard link relative to directory file descriptors.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/linkat.html
 */
int linkat(int fd1, const char *path1, int fd2, const char *path2, int flag);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Apply, test, or remove an advisory file lock (XSI extension).
 * @ingroup posix_option_group_xsi_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/lockf.html
 */
int lockf(int fildes, int function, off_t size);
#endif
/**
 * @brief Reposition the file offset of an open file descriptor.
 * @ingroup posix_option_group_fd_mgmt
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/lseek.html
 */
off_t lseek(int fildes, off_t offset, int whence);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Change the scheduling priority of the calling process (XSI extension).
 * @ingroup posix_option_group_xsi_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/nice.html
 */
int nice(int incr);
#endif
/**
 * @brief Determine the value of a configurable limit for a file path.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pathconf.html
 */
long pathconf(const char *path, int name);
/**
 * @brief Suspend process execution until a signal is received.
 * @ingroup posix_option_group_signals
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pause.html
 */
int pause(void);
/**
 * @brief Create a pipe — a pair of connected file descriptors.
 * @ingroup posix_option_group_pipe
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pipe.html
 */
int pipe(int fildes[2]);
/**
 * @brief Read from a file at a given offset without changing the file offset.
 * @ingroup posix_option_group_device_io
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pread.html
 */
ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);
/**
 * @brief Write to a file at a given offset without changing the file offset.
 * @ingroup posix_option_group_device_io
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pwrite.html
 */
ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);
/**
 * @brief Read from a file descriptor.
 * @ingroup posix_option_group_device_io
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/read.html
 */
ssize_t read(int fildes, void *buf, size_t nbyte);
/**
 * @brief Read the value of a symbolic link.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/readlink.html
 */
ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize);
/**
 * @brief Read the value of a symbolic link relative to a directory descriptor.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/readlinkat.html
 */
ssize_t readlinkat(int fd, const char *restrict path, char *restrict buf, size_t bufsize);
/**
 * @brief Remove an empty directory.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/rmdir.html
 */
int rmdir(const char *path);
/**
 * @brief Set the effective group ID of the calling process.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setegid.html
 */
int setegid(gid_t gid);
/**
 * @brief Set the effective user ID of the calling process.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/seteuid.html
 */
int seteuid(uid_t uid);
/**
 * @brief Set the real group ID of the calling process.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setgid.html
 */
int setgid(gid_t gid);
/**
 * @brief Set the process group ID for a process.
 * @ingroup posix_option_group_job_control
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setpgid.html
 */
int setpgid(pid_t pid, pid_t pgid);
#if (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500 && _XOPEN_SOURCE < 800)) ||                   \
	defined(__DOXYGEN__)
/**
 * @brief Set the process group ID to the process ID of the calling process (obsolescent).
 * @ingroup posix_option_group_job_control
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setpgrp.html
 */
pid_t setpgrp(void);
#endif
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Set the real and effective group IDs (XSI extension).
 * @ingroup posix_option_group_xsi_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setregid.html
 */
int setregid(gid_t rgid, gid_t egid);
/**
 * @brief Set the real and effective user IDs (XSI extension).
 * @ingroup posix_option_group_xsi_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setreuid.html
 */
int setreuid(uid_t ruid, uid_t euid);
#endif
/**
 * @brief Create a new session and set the process group ID.
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setsid.html
 */
pid_t setsid(void);
/**
 * @brief Set the real user ID of the calling process.
 * @ingroup posix_option_group_user_groups
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setuid.html
 */
int setuid(uid_t uid);
/**
 * @brief Suspend execution for at least the specified number of seconds.
 * @ingroup posix_option_group_multi_process
 */
unsigned int sleep(unsigned int seconds);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Swap bytes in a block of memory (XSI extension).
 * @ingroup posix_option_group_xsi_c_lang_support
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/swab.html
 */
void swab(const void *ZRESTRICT src, void *ZRESTRICT dest, ssize_t nbytes);
#endif
/**
 * @brief Create a symbolic link.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/symlink.html
 */
int symlink(const char *path1, const char *path2);
/**
 * @brief Create a symbolic link relative to a directory file descriptor.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/symlinkat.html
 */
int symlinkat(const char *path1, int fd, const char *path2);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Schedule outstanding writes to all file systems (XSI extension).
 * @ingroup posix_option_group_xsi_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sync.html
 */
void sync(void);
#endif
#ifdef CONFIG_POSIX_SYSCONF_IMPL_MACRO
/**
 * @brief Get a configurable system variable.
 * @ingroup posix_option_group_single_process
 */
#define sysconf(x) (long)CONCAT(__z_posix_sysconf, x)
#else
/**
 * @brief Get a configurable system variable.
 * @ingroup posix_option_group_single_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sysconf.html
 */
long sysconf(int opt);
#endif /* CONFIG_POSIX_SYSCONF_IMPL_FULL */
/**
 * @brief Get the process group ID of the foreground process group of the terminal.
 * @ingroup posix_option_group_job_control
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcgetpgrp.html
 */
pid_t tcgetpgrp(int fildes);
/**
 * @brief Set the foreground process group ID of a terminal.
 * @ingroup posix_option_group_job_control
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcsetpgrp.html
 */
int tcsetpgrp(int fildes, pid_t pgid_id);
/**
 * @brief Truncate a file on the filesystem to a specified length.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/truncate.html
 */
int truncate(const char *path, off_t length);
/**
 * @brief Get the name of the terminal associated with a file descriptor.
 * @ingroup posix_option_group_device_specific
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/ttyname.html
 */
char *ttyname(int fildes);
/**
 * @brief Get the name of the terminal into a caller-supplied buffer.
 * @ingroup posix_option_group_device_specific_r
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/ttyname_r.html
 */
int ttyname_r(int fildes, char *name, size_t namesize);
/**
 * @brief Remove a directory entry.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/unlink.html
 */
int unlink(const char *path);
/**
 * @brief Remove a directory entry relative to a directory file descriptor.
 * @ingroup posix_option_group_file_system
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/unlinkat.html
 */
int unlinkat(int fd, const char *path, int flag);
#if (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE < 700)) || defined(__DOXYGEN__)
/**
 * @brief Suspend execution for at least the specified number of microseconds (obsolescent).
 * @ingroup posix_option_group_multi_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/usleep.html
 */
int usleep(useconds_t useconds);
#endif
/**
 * @brief Write to a file descriptor.
 * @ingroup posix_option_group_device_io
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/write.html
 */
ssize_t write(int fildes, const void *buf, size_t nbyte);

/**
 * @brief Argument for the current option from getopt().
 * @ingroup posix_option_group_c_lib_ext
 */
extern char *optarg;
/**
 * @brief Error flag for getopt() (non-zero enables error messages).
 * @ingroup posix_option_group_c_lib_ext
 */
extern int opterr;
/**
 * @brief Index of the next element to be processed by getopt().
 * @ingroup posix_option_group_c_lib_ext
 */
extern int optind;
/**
 * @brief Unrecognised option character from getopt().
 * @ingroup posix_option_group_c_lib_ext
 */
extern int optopt;

#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif	/* ZEPHYR_INCLUDE_POSIX_UNISTD_H_ */
