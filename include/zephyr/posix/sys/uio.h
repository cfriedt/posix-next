/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX scatter/gather I/O types (<sys/uio.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_uio.h.html">
 *      POSIX.1-2017 &lt;sys/uio.h&gt;</a>
 *
 * @ingroup posix_option_group_xsi_device_io
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_UIO_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_UIO_H_

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_IOVEC_DECLARED) || defined(__iovec_defined)) || defined(__DOXYGEN__)
/** @brief Scatter/gather array element. */
struct iovec {
	void  *iov_base; /**< Pointer to data. */
	size_t iov_len;  /**< Length of the data. */
};
#define _IOVEC_DECLARED
#define __iovec_defined
#endif

/**
 * @brief Read data into multiple buffers (scatter input).
 * @param fildes Open file descriptor.
 * @param iov    Array of buffers to fill.
 * @param iovcnt Number of elements in @p iov.
 * @return Number of bytes read on success, or -1 with errno set on failure.
 * @ingroup posix_option_group_xsi_device_io
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/readv.html
 */
ssize_t readv(int fildes, const struct iovec *iov, int iovcnt);

/**
 * @brief Write data from multiple buffers (gather output).
 * @param fildes Open file descriptor.
 * @param iov    Array of buffers to write.
 * @param iovcnt Number of elements in @p iov.
 * @return Number of bytes written on success, or -1 with errno set on failure.
 * @ingroup posix_option_group_xsi_device_io
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/writev.html
 */
ssize_t writev(int fildes, const struct iovec *iov, int iovcnt);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_UIO_H_ */
