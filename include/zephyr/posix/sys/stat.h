/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX file status types and functions (<sys/stat.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_stat.h.html">
 *      POSIX.1-2017 &lt;sys/stat.h&gt;</a>
 *
 * @defgroup posix_stat POSIX File Status
 * @ingroup posix_option_group_file_system
 * @{
 */

#ifndef ZEPHYR_INCLUDE_ZEPHYR_POSIX_SYS_STAT_H_
#define ZEPHYR_INCLUDE_ZEPHYR_POSIX_SYS_STAT_H_

#include <time.h>

#include <zephyr/toolchain.h>
#include <zephyr/sys/fdtable.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

/* slightly out of order w.r.t. the specification */
#if !defined(_BLKCNT_T_DECLARED) && !defined(__blkcnt_t_defined)
typedef long blkcnt_t;
#define _BLKCNT_T_DECLARED
#define __blkcnt_t_defined
#endif

#if !defined(_BLKSIZE_T_DECLARED) && !defined(__blksize_t_defined)
typedef unsigned long blksize_t;
#define _BLKSIZE_T_DECLARED
#define __blksize_t_defined
#endif

#if !defined(_DEV_T_DECLARED) && !defined(__dev_t_defined)
typedef int dev_t;
#define _DEV_T_DECLARED
#define __dev_t_defined
#endif

#if !defined(_GID_T_DECLARED) && !defined(__gid_t_defined)
typedef unsigned short gid_t;
#define _GID_T_DECLARED
#define __gid_t_defined
#endif

#if !defined(_INO_T_DECLARED) && !defined(__ino_t_defined)
typedef long ino_t;
#define _INO_T_DECLARED
#define __ino_t_defined
#endif

#if !defined(_MODE_T_DECLARED) && !defined(__mode_t_defined)
typedef int mode_t;
#define _MODE_T_DECLARED
#define __mode_t_defined
#endif

#if !defined(_NLINK_T_DECLARED) && !defined(__nlink_t_defined)
typedef unsigned short nlink_t;
#define _NLINK_T_DECLARED
#define __nlink_t_defined
#endif

#if !defined(_OFF_T_DECLARED) && !defined(__off_t_defined)
typedef long off_t;
#define _OFF_T_DECLARED
#define __off_t_defined
#endif

/* time_t must be defined by the libc time.h */
#include <time.h>

#if __STDC_VERSION__ >= 201112L
/* struct timespec must be defined in the libc time.h */
#else
/*
 * there is a workaround needed for picolibc because it doesn't have guards around the definition
 * of struct timespec
 */
#if !defined(_TIMESPEC_DECLARED) && !defined(__timespec_defined) && !defined(CONFIG_PICOLIBC)
struct timespec {
	time_t tv_sec;
	long tv_nsec;
};
#define _TIMESPEC_DECLARED
#define __timespec_defined
#endif
#endif

#if !defined(_UID_T_DECLARED) && !defined(__uid_t_defined)
typedef unsigned short uid_t;
#define _UID_T_DECLARED
#define __uid_t_defined
#endif

#if !(defined(_STAT_DECLARED) || defined(__stat_defined)) || defined(__DOXYGEN__)
/** @brief File status information returned by stat(), fstat(), and lstat(). */
struct stat {
	dev_t st_dev;             /**< Device ID of the device containing the file. */
	ino_t st_ino;             /**< File serial number (inode). */
	mode_t st_mode;           /**< File type and permission bits (S_IF* and S_I* constants). */
	nlink_t st_nlink;         /**< Number of hard links. */
	uid_t st_uid;             /**< User ID of the file owner. */
	gid_t st_gid;             /**< Group ID of the file's group. */
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
	dev_t st_rdev;            /**< Device ID (for character/block special files). */
#endif
	off_t st_size;            /**< File size in bytes (regular files). */
	struct timespec st_atim;  /**< Time of last access. */
	struct timespec st_mtim;  /**< Time of last data modification. */
	struct timespec st_ctim;  /**< Time of last status change. */
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
	blkcnt_t st_blksize;      /**< Preferred I/O block size. */
	blkcnt_t st_blocks;       /**< Number of 512-byte blocks allocated. */
#endif
};
#define _STAT_DECLARED
#define __stat_defined
#endif

/** @brief Bit mask for the file type bits in st_mode. */
#define S_IFMT ZVFS_MODE_IFMT

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Block special file. */
#define S_IFBLK  ZVFS_MODE_IFBLK
/** @brief Character special file. */
#define S_IFCHR  ZVFS_MODE_IFCHR
/** @brief FIFO special file. */
#define S_IFIFO  ZVFS_MODE_IFIFO
/** @brief Regular file. */
#define S_IFREG  ZVFS_MODE_IFREG
/** @brief Directory. */
#define S_IFDIR  ZVFS_MODE_IFDIR
/** @brief Symbolic link. */
#define S_IFLNK  ZVFS_MODE_IFLNK
/** @brief Socket. */
#define S_IFSOCK ZVFS_MODE_IFSOCK
/** @brief Shared memory object (Zephyr extension). */
#define S_IFSHM  ZVFS_MODE_IFSHM
#endif

/** @brief Read, write, execute permission for owner. */
#define S_IRWXU ZVFS_S_IRWXU
/** @brief Read permission for owner. */
#define S_IRUSR ZVFS_S_IRUSR
/** @brief Write permission for owner. */
#define S_IWUSR ZVFS_S_IWUSR
/** @brief Execute permission for owner. */
#define S_IXUSR ZVFS_S_IXUSR
/** @brief Read, write, execute permission for group. */
#define S_IRWXG ZVFS_S_IRWXG
/** @brief Read permission for group. */
#define S_IRGRP ZVFS_S_IRGRP
/** @brief Write permission for group. */
#define S_IWGRP ZVFS_S_IWGRP
/** @brief Execute permission for group. */
#define S_IXGRP ZVFS_S_IXGRP
/** @brief Read, write, execute permission for others. */
#define S_IRWXO ZVFS_S_IRWXO
/** @brief Read permission for others. */
#define S_IROTH ZVFS_S_IROTH
/** @brief Write permission for others. */
#define S_IWOTH ZVFS_S_IWOTH
/** @brief Execute permission for others. */
#define S_IXOTH ZVFS_S_IXOTH
/** @brief Set-user-ID bit. */
#define S_ISUID ZVFS_S_ISUID
/** @brief Set-group-ID bit. */
#define S_ISGID ZVFS_S_ISGID
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Sticky bit (restricted deletion). */
#define S_ISVTX ZVFS_S_ISVTX
#endif

/** @brief Test whether @p m is a block special file. */
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Test whether @p m is a character special file. */
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
/** @brief Test whether @p m is a directory. */
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
/** @brief Test whether @p m is a FIFO. */
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
/** @brief Test whether @p m is a regular file. */
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
/** @brief Test whether @p m is a symbolic link. */
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
/** @brief Test whether @p m is a socket. */
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Test whether the file is a message queue (always 0). */
#define S_TYPEISMQ(buf)  (0)
/** @brief Test whether the file is a semaphore (always 0). */
#define S_TYPEISSEM(buf) (0)
/** @brief Test whether the file is a shared memory object. */
#define S_TYPEISSHM(st)  (((st)->st_mode & S_IFMT) == S_IFSHM)
#endif

#if defined(_POSIX_TYPED_MEMORY_OBJECTS) || defined(__DOXYGEN__)
/** @brief Test whether the file is a typed memory object (always 0). */
#define S_TYPEISTMO(buf) (0)
#endif

/** @brief Set st_atim or st_mtim to the current time (utimensat flag). */
#define UTIME_NOW  -1
/** @brief Leave st_atim or st_mtim unchanged (utimensat flag). */
#define UTIME_OMIT -2

/** @brief Change the mode of a file. */
int chmod(const char *path, mode_t mode);
/** @brief Change the mode of an open file. */
int fchmod(int fildes, mode_t mode);
/** @brief Change the mode of a file relative to a directory descriptor. */
int fchmodat(int fd, const char *path, mode_t mode, int flag);
/** @brief Get status of an open file. */
int fstat(int fildes, struct stat *buf);
/** @brief Get status of a file relative to a directory descriptor. */
int fstatat(int fd, const char *ZRESTRICT path, struct stat *ZRESTRICT buf, int flag);
/** @brief Set file access and modification times of an open file (nanosecond resolution). */
int futimens(int fildes, const struct timespec times[2]);
/** @brief Get status of a file (does not follow symbolic links). */
int lstat(const char *ZRESTRICT path, struct stat *ZRESTRICT buf);
/** @brief Create a directory. */
int mkdir(const char *path, mode_t mode);
/** @brief Create a directory relative to a directory descriptor. */
int mkdirat(int fd, const char *path, mode_t mode);
/** @brief Create a FIFO special file. */
int mkfifo(const char *path, mode_t mode);
/** @brief Create a FIFO special file relative to a directory descriptor. */
int mkfifoat(int fd, const char *path, mode_t mode);
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Create a special or regular file (XSI extension). */
int mknod(const char *path, mode_t mode, dev_t dev);
/** @brief Create a special or regular file relative to a directory descriptor (XSI). */
int mknodat(int fd, const char *path, mode_t mode, dev_t dev);
#endif
TOOLCHAIN_DISABLE_WARNING(TOOLCHAIN_WARNING_SHADOW);
/** @brief Get status of a file by path (follows symbolic links). */
int stat(const char *ZRESTRICT path, struct stat *ZRESTRICT buf);
TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_SHADOW);
/** @brief Set the file mode creation mask. */
mode_t umask(mode_t cmask);
/** @brief Set file access and modification times relative to a directory descriptor. */
int utimensat(int fd, const char *path, const struct timespec times[2], int flag);

/** @} */ /* posix_stat */

#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_ZEPHYR_POSIX_SYS_STAT_H_ */
