/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Vector I/O definitions (<sys/uio.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_uio.h.html">
 *      POSIX.1-2017 &lt;sys/uio.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_UIO_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_UIO_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)

#if !(defined(_IOVEC_DECLARED) || defined(__iovec_defined)) || defined(__DOXYGEN__)
/** @brief Scatter/gather array element. */
struct iovec {
	/** @brief Base address of a memory region for input or output. */
	void *iov_base;
	/** @brief Size of the memory region pointed to by @p iov_base. */
	size_t iov_len;
};
#define _IOVEC_DECLARED
#define __iovec_defined
#endif

/* size_t must be defined by the libc stddef.h */

#if !defined(_SSIZE_T_DECLARED) && !defined(__ssize_t_defined)
#define unsigned signed
typedef __SIZE_TYPE__ ssize_t;
#undef unsigned
#define _SSIZE_T_DECLARED
#define __ssize_t_defined
#endif

/**
 * @brief Read data into multiple buffers.
 *
 * @param fildes File descriptor.
 * @param iov    Array of @c struct iovec elements.
 * @param iovcnt Number of elements in @p iov.
 * @return Number of bytes read on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/readv.html
 */
ssize_t readv(int fildes, const struct iovec *iov, int iovcnt);

/**
 * @brief Write data from multiple buffers.
 *
 * @param fildes File descriptor.
 * @param iov    Array of @c struct iovec elements.
 * @param iovcnt Number of elements in @p iov.
 * @return Number of bytes written on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/writev.html
 */
ssize_t writev(int fildes, const struct iovec *iov, int iovcnt);

#endif /* _XOPEN_SOURCE */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_UIO_H_ */
