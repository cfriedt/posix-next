/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Linux kernel ABI types
 *
 * The scalar and aggregate types of the Linux system call interface, as the
 * kernel defines them for user space (include/uapi), under zkcl_linux_ names so
 * that they never collide with the C library's.
 */

#ifndef ZEPHYR_INCLUDE_ZEPHYR_KCL_LINUX_TYPES_H_
#define ZEPHYR_INCLUDE_ZEPHYR_KCL_LINUX_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int zkcl_linux_pid_t;
typedef unsigned int zkcl_linux_uid_t;
typedef unsigned int zkcl_linux_gid_t;
typedef unsigned short zkcl_linux_old_uid_t;
typedef unsigned short zkcl_linux_old_gid_t;
typedef unsigned short zkcl_linux_umode_t;
typedef long zkcl_linux_off_t;
typedef int64_t zkcl_linux_loff_t;
typedef long zkcl_linux_old_time_t;
typedef int32_t zkcl_linux_old_time32_t;
typedef int zkcl_linux_clockid_t;
typedef int zkcl_linux_timer_t;
typedef int zkcl_linux_key_t;
typedef int32_t zkcl_linux_key_serial_t;
typedef int zkcl_linux_mqd_t;
typedef unsigned int zkcl_linux_qid_t;
typedef int zkcl_linux_rwf_t;
typedef unsigned long zkcl_linux_aio_context_t;
typedef unsigned long zkcl_linux_old_sigset_t;
typedef void (*zkcl_linux_sighandler_t)(int);

/** Signal set: 64 signals, as words of the native size */
typedef struct {
	unsigned long sig[64 / (8 * sizeof(unsigned long))];
} zkcl_linux_sigset_t;

/** Descriptor set of select(): 1024 bits */
typedef struct {
	unsigned long fds_bits[1024 / (8 * sizeof(unsigned long))];
} zkcl_linux_fd_set;

/** Signal information: 128 bytes, the fields the layer fills up front */
typedef struct zkcl_linux_siginfo {
	int si_signo;
	int si_errno;
	int si_code;
	int _pad0;
	union {
		uint8_t _pad[128 - 4 * sizeof(int)];
		struct {
			zkcl_linux_pid_t pid;
			zkcl_linux_uid_t uid;
			int status;
		} kill;
	} _sifields;
} zkcl_linux_siginfo_t;

struct zkcl_linux_user_cap_header {
	uint32_t version;
	int pid;
};

struct zkcl_linux_user_cap_data {
	uint32_t effective;
	uint32_t permitted;
	uint32_t inheritable;
};

typedef struct zkcl_linux_user_cap_header *zkcl_linux_cap_user_header_t;
typedef struct zkcl_linux_user_cap_data *zkcl_linux_cap_user_data_t;

struct zkcl_linux_iovec {
	void *iov_base;
	size_t iov_len;
};

struct zkcl_linux_timespec {
	int64_t tv_sec;
	long long tv_nsec;
};

struct zkcl_linux_old_timeval {
	long tv_sec;
	long tv_usec;
};

struct zkcl_linux_timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

struct zkcl_linux_itimerspec {
	struct zkcl_linux_timespec it_interval;
	struct zkcl_linux_timespec it_value;
};

struct zkcl_linux_rlimit64 {
	uint64_t rlim_cur;
	uint64_t rlim_max;
};

struct zkcl_linux_rlimit {
	unsigned long rlim_cur;
	unsigned long rlim_max;
};

#define ZKCL_LINUX_UTSNAME_LENGTH 65

struct zkcl_linux_new_utsname {
	char sysname[ZKCL_LINUX_UTSNAME_LENGTH];
	char nodename[ZKCL_LINUX_UTSNAME_LENGTH];
	char release[ZKCL_LINUX_UTSNAME_LENGTH];
	char version[ZKCL_LINUX_UTSNAME_LENGTH];
	char machine[ZKCL_LINUX_UTSNAME_LENGTH];
	char domainname[ZKCL_LINUX_UTSNAME_LENGTH];
};

/** stat() result: the x86_64 layout, else the asm-generic one */
struct zkcl_linux_stat {
#if defined(CONFIG_X86_64)
	unsigned long st_dev;
	unsigned long st_ino;
	unsigned long st_nlink;
	unsigned int st_mode;
	unsigned int st_uid;
	unsigned int st_gid;
	unsigned int _pad0;
	unsigned long st_rdev;
	long st_size;
	long st_blksize;
	long st_blocks;
	unsigned long st_atime;
	unsigned long st_atime_nsec;
	unsigned long st_mtime;
	unsigned long st_mtime_nsec;
	unsigned long st_ctime;
	unsigned long st_ctime_nsec;
	long _unused[3];
#else
	unsigned long st_dev;
	unsigned long st_ino;
	unsigned int st_mode;
	unsigned int st_nlink;
	unsigned int st_uid;
	unsigned int st_gid;
	unsigned long st_rdev;
	unsigned long _pad1;
	long st_size;
	int st_blksize;
	int _pad2;
	long st_blocks;
	long st_atime;
	unsigned long st_atime_nsec;
	long st_mtime;
	unsigned long st_mtime_nsec;
	long st_ctime;
	unsigned long st_ctime_nsec;
	unsigned int _unused4;
	unsigned int _unused5;
#endif
};

struct zkcl_linux_sigaction {
	zkcl_linux_sighandler_t sa_handler;
	unsigned long sa_flags;
	void (*sa_restorer)(void);
	zkcl_linux_sigset_t sa_mask;
};

struct zkcl_linux_sigaltstack {
	void *ss_sp;
	int ss_flags;
	size_t ss_size;
};

struct zkcl_linux_robust_list_head {
	void *list;
	long futex_offset;
	void *list_op_pending;
};

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_ZEPHYR_KCL_LINUX_TYPES_H_ */
