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
 * @ingroup posix_option_group_networking
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

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_UIO_H_ */
