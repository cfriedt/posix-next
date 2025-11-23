/*
 * SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
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
typedef int nfds_t;
#define _NFDS_T_DECLARED
#define __nfds_t_defined
#endif

#define pollfd zvfs_pollfd

#define POLLIN     ZVFS_POLLIN
#define POLLRDNORM ZVFS_POLLRDNORM
#define POLLRDBAND ZVFS_POLLRDBAND
#define POLLPRI    ZVFS_POLLPRI
#define POLLOUT    ZVFS_POLLOUT
#define POLLWRNORM ZVFS_POLLWRNORM
#define POLLWRBAND ZVFS_POLLWRBAND
#define POLLERR    ZVFS_POLLERR
#define POLLHUP    ZVFS_POLLHUP
#define POLLNVAL   ZVFS_POLLNVAL

BUILD_ASSERT(POLLWRNORM == POLLOUT, "POLLWRNORM must be equal to POLLOUT");

int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif /* _POSIX_C_SOURCE || __DOXYGEN__ */

#ifdef __cplusplus
}
#endif

#endif	/* ZEPHYR_INCLUDE_POSIX_POLL_H_ */
