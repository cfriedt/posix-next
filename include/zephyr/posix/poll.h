/*
 * SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX poll API (<poll.h>)
 *
 * Provides the poll() function and associated poll event flags for
 * multiplexed I/O on multiple file descriptors.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/poll.h.html">
 *      POSIX.1-2017 &lt;poll.h&gt;</a>
 *
 * @ingroup posix_option_group_device_io
 */

#ifndef ZEPHYR_INCLUDE_POSIX_POLL_H_
#define ZEPHYR_INCLUDE_POSIX_POLL_H_

#include <zephyr/sys/fdtable.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#if !defined(_NFDS_T_DECLARED) && !defined(__nfds_t_defined)
/** @brief Type for the number of file descriptors passed to poll().  @ingroup posix_option_group_device_io*/
typedef int nfds_t;
#define _NFDS_T_DECLARED
#define __nfds_t_defined
#endif

/** @brief Poll file descriptor structure (alias for zvfs_pollfd).  @ingroup posix_option_group_device_io*/
#define pollfd zvfs_pollfd

/** @brief Data other than high-priority data may be read without blocking.  @ingroup posix_option_group_device_io*/
#define POLLIN     ZVFS_POLLIN
/** @brief Normal data may be read without blocking.  @ingroup posix_option_group_device_io*/
#define POLLRDNORM ZVFS_POLLRDNORM
/** @brief Priority data may be read without blocking.  @ingroup posix_option_group_device_io*/
#define POLLRDBAND ZVFS_POLLRDBAND
/** @brief High-priority data may be read without blocking.  @ingroup posix_option_group_device_io*/
#define POLLPRI    ZVFS_POLLPRI
/** @brief Normal data may be written without blocking.  @ingroup posix_option_group_device_io*/
#define POLLOUT    ZVFS_POLLOUT
/** @brief Equivalent to POLLOUT.  @ingroup posix_option_group_device_io*/
#define POLLWRNORM ZVFS_POLLWRNORM
/** @brief Priority data may be written.  @ingroup posix_option_group_device_io*/
#define POLLWRBAND ZVFS_POLLWRBAND
/** @brief An error has occurred on the file descriptor (output only).  @ingroup posix_option_group_device_io*/
#define POLLERR    ZVFS_POLLERR
/** @brief The file descriptor has been hung up (output only).  @ingroup posix_option_group_device_io*/
#define POLLHUP    ZVFS_POLLHUP
/** @brief The file descriptor is not open (output only).  @ingroup posix_option_group_device_io*/
#define POLLNVAL   ZVFS_POLLNVAL

BUILD_ASSERT(POLLWRNORM == POLLOUT, "POLLWRNORM must be equal to POLLOUT");

/**
 * @brief Wait for events on a set of file descriptors.
 * @ingroup posix_option_group_device_io
 *
 * Examines each file descriptor in @p fds for the events specified in its
 * @c events field.  Blocks until at least one descriptor is ready, the
 * timeout expires, or a signal is caught.
 *
 * @param fds     Array of @c struct pollfd descriptors to monitor.
 * @param nfds    Number of entries in @p fds.
 * @param timeout Timeout in milliseconds; -1 to block indefinitely, 0 to return immediately.
 * @return Number of file descriptors with non-zero @c revents on success,
 *         0 on timeout, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/poll.html
 */
int poll(struct pollfd *fds, nfds_t nfds, int timeout);


#endif /* _POSIX_C_SOURCE || __DOXYGEN__ */

#ifdef __cplusplus
}
#endif

#endif	/* ZEPHYR_INCLUDE_POSIX_POLL_H_ */
