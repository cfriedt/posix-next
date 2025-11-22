/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
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

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1

/* confstr() constants in zephyr/posix/sys/confstr.h */

/* SEEK_CUR, SEEK_END, SEEK_SET nominally defined in <stdio.h> */
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)

#define F_LOCK  1
#define F_TEST  3
#define F_TLOCK 2
#define F_ULOCK 0

#endif

/* pathconf() constants in zephyr/posix/sys/pathconf.h  */
/* sysconf() constants in zephyr/posix/sys/sysconf.h  */

#define STDERR_FILENO 2
#define STDIN_FILENO  0
#define STDOUT_FILENO 1

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

int access(const char *path, int amode);
unsigned int alarm(unsigned int seconds);
int chdir(const char *path);
int chown(const char *path, uid_t owner, gid_t group);
int close(int fildes);
size_t confstr(int name, char *buf, size_t len);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
char *crypt(const char *key, const char *salt);
#endif
int dup(int fildes);
int dup2(int fildes, int fildes2);
FUNC_NORETURN void _exit(int status);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
void encrypt(char block[64], int edflag);
#endif
int execl(const char *path, const char *arg0, ...);
int execle(const char *path, const char *arg0, ...);
int execlp(const char *file, const char *arg0, ...);
int execv(const char *path, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
int faccessat(int fd, const char *path, int amode, int flag);
int fchdir(int fildes);
int fchown(int fildes, uid_t owner, gid_t group);
int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flag);
#if defined(_POSIX_SYNCHRONIZED_IO) || defined(__DOXYGEN__)
int fdatasync(int fildes);
#endif
int fexecve(int fd, char *const argv[], char *const envp[]);
pid_t fork(void);
long fpathconf(int fildes, int name);
#if defined(_POSIX_FSYNC) || defined(__DOXYGEN__)
int fsync(int fd);
#endif
int ftruncate(int fildes, off_t length);
char *getcwd(char *buf, size_t size);
gid_t getegid(void);
#if (_POSIX_C_SOURCE >= 202405L) || defined(__DOXYGEN__)
int getentropy(void *buffer, size_t length);
#endif
uid_t geteuid(void);
gid_t getgid(void);
int getgroups(int gidsetsize, gid_t grouplist[]);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
long gethostid(void);
#endif
int gethostname(char *name, size_t namelen);
char *getlogin(void);
int getlogin_r(char *name, size_t namesize);
int getopt(int argc, char *const argv[], const char *optstring);
pid_t getpgid(pid_t pid);
pid_t getpgrp(void);
pid_t getpid(void);
pid_t getppid(void);
pid_t getsid(pid_t pid);
uid_t getuid(void);
int isatty(int fildes);
int lchown(const char *path, uid_t owner, gid_t group);
int link(const char *path1, const char *path2);
int linkat(int fd1, const char *path1, int fd2, const char *path2, int flag);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
int lockf(int fildes, int function, off_t size);
#endif
off_t lseek(int fildes, off_t offset, int whence);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
int nice(int incr);
#endif
long pathconf(const char *path, int name);
int pause(void);
int pipe(int fildes[2]);
ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);
ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);
ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize);
ssize_t readlinkat(int fd, const char *restrict path, char *restrict buf, size_t bufsize);
int rmdir(const char *path);
int setegid(gid_t gid);
int seteuid(uid_t uid);
int setgid(gid_t gid);
int setpgid(pid_t pid, pid_t pgid);
#if (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500 && _XOPEN_SOURCE < 800)) ||                   \
	defined(__DOXYGEN__)
pid_t setpgrp(void);
#endif
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
int setregid(gid_t rgid, gid_t egid);
int setreuid(uid_t ruid, uid_t euid);
#endif
pid_t setsid(void);
int setuid(uid_t uid);
unsigned int sleep(unsigned int seconds);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
void swab(const void *ZRESTRICT src, void *ZRESTRICT dest, ssize_t nbytes);
#endif
int symlink(const char *path1, const char *path2);
int symlinkat(const char *path1, int fd, const char *path2);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
void sync(void);
#endif
#ifdef CONFIG_POSIX_SYSCONF_IMPL_MACRO
#define sysconf(x) (long)CONCAT(__z_posix_sysconf, x)
#else
long sysconf(int opt);
#endif /* CONFIG_POSIX_SYSCONF_IMPL_FULL */
pid_t tcgetpgrp(int fildes);
int tcsetpgrp(int fildes, pid_t pgid_id);
int truncate(const char *path, off_t length);
char *ttyname(int fildes);
int ttyname_r(int fildes, char *name, size_t namesize);
int unlink(const char *path);
int unlinkat(int fd, const char *path, int flag);
#if (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE < 700)) || defined(__DOXYGEN__)
int usleep(useconds_t useconds);
#endif
ssize_t write(int fildes, const void *buf, size_t nbyte);

extern char *optarg;
extern int opterr, optind, optopt;

#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif	/* ZEPHYR_INCLUDE_POSIX_UNISTD_H_ */
