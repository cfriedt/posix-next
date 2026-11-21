/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief \<sys/resource.h\>: resource operations
 *
 * Resource limits and usage: the XSI_MULTI_PROCESS option group.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_resource.h.html">
 *      POSIX.1-2017 &lt;sys/resource.h&gt;</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_RESOURCE_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_RESOURCE_H_

#include <sys/time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup posix_option_group_xsi_multi_process XSI_MULTI_PROCESS
 * @ingroup posix_apis
 * @{
 */

#if !defined(_ID_T_DECLARED) && !defined(__id_t_defined)
typedef int id_t; /**< A process, process group or user ID. */
#define _ID_T_DECLARED
#define __id_t_defined
#endif

/** @brief A resource limit value. */
typedef unsigned long long rlim_t;

/** @brief No limit. */
#define RLIM_INFINITY  ((rlim_t)-1)
/** @brief An unrepresentable saved hard limit. */
#define RLIM_SAVED_MAX RLIM_INFINITY
/** @brief An unrepresentable saved soft limit. */
#define RLIM_SAVED_CUR RLIM_INFINITY

/** @name Resources for getrlimit() and setrlimit() */
/**@{*/
#define RLIMIT_CPU     0 /**< CPU time, in seconds. */
#define RLIMIT_FSIZE   1 /**< Largest file that may be created, in bytes. */
#define RLIMIT_DATA    2 /**< Data segment size, in bytes. */
#define RLIMIT_STACK   3 /**< Stack size, in bytes. */
#define RLIMIT_CORE    4 /**< Largest core file, in bytes; 0 writes none. */
#define RLIMIT_RSS     5 /**< Resident set size, in bytes. */
#define RLIMIT_NPROC   6 /**< Number of processes. */
#define RLIMIT_NOFILE  7 /**< One more than the largest file descriptor. */
#define RLIMIT_MEMLOCK 8 /**< Locked memory, in bytes. */
#define RLIMIT_AS      9 /**< Address space size, in bytes. */
#define RLIM_NLIMITS   10 /**< Number of resources. */
/**@}*/

/** @brief A pair of limits on a resource. */
struct rlimit {
	rlim_t rlim_cur; /**< The current (soft) limit. */
	rlim_t rlim_max; /**< The hard limit. */
};

/** @name Targets of getpriority() and setpriority() */
/**@{*/
#define PRIO_PROCESS 0 /**< A process. */
#define PRIO_PGRP    1 /**< A process group. */
#define PRIO_USER    2 /**< A user. */
/**@}*/

/** @name Targets of getrusage() */
/**@{*/
#define RUSAGE_SELF     0  /**< The calling process. */
#define RUSAGE_CHILDREN -1 /**< The calling process's terminated and waited-for children. */
/**@}*/

/**
 * @brief Resource usage.
 *
 * POSIX names the two times; the other members are the ones every system reports, so a program
 * written for one compiles. The kernel counts none of them, so they are zero.
 */
struct rusage {
	struct timeval ru_utime; /**< User CPU time used. */
	struct timeval ru_stime; /**< System CPU time used. */
	long ru_maxrss;          /**< Maximum resident set size, in kilobytes. */
	long ru_ixrss;           /**< Shared memory size. */
	long ru_idrss;           /**< Unshared data size. */
	long ru_isrss;           /**< Unshared stack size. */
	long ru_minflt;          /**< Page faults served without I/O. */
	long ru_majflt;          /**< Page faults served with I/O. */
	long ru_nswap;           /**< Swaps. */
	long ru_inblock;         /**< Block input operations. */
	long ru_oublock;         /**< Block output operations. */
	long ru_msgsnd;          /**< Messages sent. */
	long ru_msgrcv;          /**< Messages received. */
	long ru_nsignals;        /**< Signals received. */
	long ru_nvcsw;           /**< Voluntary context switches. */
	long ru_nivcsw;          /**< Involuntary context switches. */
};

/**
 * @brief Get the scheduling priority of a process, process group or user.
 * @param which One of PRIO_PROCESS, PRIO_PGRP or PRIO_USER.
 * @param who The ID, 0 for the caller's.
 * @return The priority, or -1 with errno set.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpriority.html
 */
int getpriority(int which, id_t who);

/**
 * @brief Get the limits on a resource.
 * @param resource One of the RLIMIT_* resources.
 * @param rlp Destination for the limits.
 * @return 0, or -1 with errno set.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getrlimit.html
 */
int getrlimit(int resource, struct rlimit *rlp);

/**
 * @brief Get the resource usage of the calling process or of its waited-for children.
 * @param who RUSAGE_SELF or RUSAGE_CHILDREN.
 * @param r_usage Destination for the usage.
 * @return 0, or -1 with errno set.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getrusage.html
 */
int getrusage(int who, struct rusage *r_usage);

/**
 * @brief Set the scheduling priority of a process, process group or user.
 * @param which One of PRIO_PROCESS, PRIO_PGRP or PRIO_USER.
 * @param who The ID, 0 for the caller's.
 * @param value The priority.
 * @return 0, or -1 with errno set.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setpriority.html
 */
int setpriority(int which, id_t who, int value);

/**
 * @brief Set the limits on a resource.
 * @param resource One of the RLIMIT_* resources.
 * @param rlp The limits to set.
 * @return 0, or -1 with errno set.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setrlimit.html
 */
int setrlimit(int resource, const struct rlimit *rlp);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_RESOURCE_H_ */
