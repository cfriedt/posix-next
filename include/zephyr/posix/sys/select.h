/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX select API (<sys/select.h>)
 *
 * Provides the select() and pselect() functions and the associated
 * fd_set type and macros for multiplexed I/O.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_select.h.html">
 *      POSIX.1-2017 &lt;sys/select.h&gt;</a>
 *
 * @defgroup posix_select POSIX select
 * @ingroup posix_option_group_file_system
 * @{
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_SELECT_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_SELECT_H_

#include <zephyr/sys/fdtable.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum number of file descriptors in an fd_set. */
#define FD_SETSIZE ZVFS_FD_SETSIZE

#if !defined(_SIGSET_T_DECLARED) && !defined(__sigset_t_defined)

#ifndef SIGRTMIN
#define SIGRTMIN (CONFIG_THREAD_CANCEL_SIGNAL_NUMBER + 2)
#endif
#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)
BUILD_ASSERT(CONFIG_POSIX_RTSIG_MAX >= 0);
/** @brief Largest real-time signal number. */
#define SIGRTMAX (SIGRTMIN + CONFIG_POSIX_RTSIG_MAX)
#else
#define SIGRTMAX SIGRTMIN
#endif

/** @brief Set of signals (bitmask). */
typedef struct {
	unsigned long sig[DIV_ROUND_UP(SIGRTMAX + 1, BITS_PER_LONG)];
} sigset_t;
#define _SIGSET_T_DECLARED
#define __sigset_t_defined
#endif

#if !defined(_SUSECONDS_T_DECLARED) && !defined(__suseconds_t_defined)
/** @brief Microsecond time component type. */
typedef long suseconds_t;
#define _SUSECONDS_T_DECLARED
#define __suseconds_t_defined
#endif

/* time_t must be defined by the libc time.h */
#include <time.h>

#if __STDC_VERSION__ >= 201112L
/* struct timespec must be defined in the libc time.h */
#else
#if !defined(_TIMESPEC_DECLARED) && !defined(__timespec_defined)
/** @brief Time value with nanosecond resolution. */
struct timespec {
	time_t tv_sec; /**< Seconds. */
	long tv_nsec;  /**< Nanoseconds [0, 999999999]. */
};
#define _TIMESPEC_DECLARED
#define __timespec_defined
#endif
#endif

#if !defined(_TIMEVAL_DECLARED) && !defined(__timeval_defined)
/** @brief Time value with microsecond resolution. */
struct timeval {
	time_t tv_sec;       /**< Seconds. */
	suseconds_t tv_usec; /**< Microseconds [0, 999999]. */
};
#define _TIMEVAL_DECLARED
#define __timeval_defined
#endif

/** @brief Set of file descriptors for select()/pselect(). */
typedef struct zvfs_fd_set fd_set;

struct timeval;

/**
 * @brief Synchronous multiplexed I/O with signal-mask and nanosecond timeout.
 * @param nfds    Highest-numbered file descriptor in any set + 1.
 * @param readfds   Set of file descriptors to watch for readability, or NULL.
 * @param writefds  Set of file descriptors to watch for writability, or NULL.
 * @param exceptfds Set of file descriptors to watch for exceptions, or NULL.
 * @param timeout   Maximum wait time, or NULL to block indefinitely.
 * @param sigmask   Signal mask to apply during the wait, or NULL.
 * @return Number of ready file descriptors on success, 0 on timeout,
 *         or -1 with errno set on failure.
 */
int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	    const struct timespec *timeout, const sigset_t *sigmask);

/**
 * @brief Synchronous multiplexed I/O (legacy interface, use pselect() for new code).
 * @param nfds      Highest-numbered file descriptor in any set + 1.
 * @param readfds   Set of file descriptors to watch for readability, or NULL.
 * @param writefds  Set of file descriptors to watch for writability, or NULL.
 * @param errorfds  Set of file descriptors to watch for errors, or NULL.
 * @param timeout   Maximum wait time, or NULL to block indefinitely.
 * @return Number of ready file descriptors on success, 0 on timeout,
 *         or -1 with errno set on failure.
 */
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);

/**
 * @brief Remove a file descriptor from an fd_set.
 * @param fd    File descriptor to clear.
 * @param fdset File descriptor set to modify.
 */
void FD_CLR(int fd, fd_set *fdset);

/**
 * @brief Test whether a file descriptor is in an fd_set.
 * @param fd    File descriptor to test.
 * @param fdset File descriptor set.
 * @return Non-zero if @p fd is set, 0 otherwise.
 */
int FD_ISSET(int fd, fd_set *fdset);

/**
 * @brief Add a file descriptor to an fd_set.
 * @param fd    File descriptor to add.
 * @param fdset File descriptor set to modify.
 */
void FD_SET(int fd, fd_set *fdset);

/**
 * @brief Clear all file descriptors from an fd_set.
 * @param fdset File descriptor set to clear.
 */
void FD_ZERO(fd_set *fdset);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_SELECT_H_ */
