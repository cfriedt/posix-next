/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX device I/O control (<sys/ioctl.h>)
 *
 * Provides ioctl() for performing device-specific control operations on file
 * descriptors, along with the standard non-blocking and pending-read flags.
 *
 * @note sys/ioctl.h is not part of the strict POSIX.1-2017 standard but is a
 *       widely-supported extension available on Linux and BSD systems.
 *
 * @ingroup posix_option_group_xsi_streams
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_IOCTL_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_IOCTL_H_

#include <zephyr/sys/fdtable.h>

/** @brief Set or clear non-blocking I/O mode on a file descriptor.  @ingroup posix_option_group_xsi_streams*/
#define FIONBIO  ZFD_IOCTL_FIONBIO
/** @brief Return the number of bytes available to read without blocking.  @ingroup posix_option_group_xsi_streams*/
#define FIONREAD ZFD_IOCTL_FIONREAD

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Perform a device-specific control operation.
 * @ingroup posix_option_group_xsi_streams
 * @param fd      File descriptor.
 * @param request Implementation-defined request code.
 * @param ...     Optional argument (typically a pointer or int) whose type
 *                is determined by @p request.
 * @return Request-specific return value on success, or -1 with errno set.
 */
int ioctl(int fd, unsigned long request, ...);


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_IOCTL_H_ */
