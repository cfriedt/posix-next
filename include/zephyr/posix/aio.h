/*
 * Copyright 2024 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX asynchronous I/O (<aio.h>)
 *
 * Provides the aiocb control block and the asynchronous I/O functions that
 * allow read, write, and fsync operations to proceed in the background.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/aio.h.html">
 *      POSIX.1-2017 &lt;aio.h&gt;</a>
 *
 * @ingroup posix_option_asynchronous_io
 */

#ifndef ZEPHYR_INCLUDE_ZEPHYR_POSIX_AIO_H_
#define ZEPHYR_INCLUDE_ZEPHYR_POSIX_AIO_H_

/* size_t must be defined by the libc stddef.h */
#include <stddef.h>
#include <stdint.h>
#include <signal.h>

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_OFF_T_DECLARED) && !defined(__off_t_defined)
/** @brief File offset type.  @ingroup posix_option_asynchronous_io*/
typedef long off_t;
#define _OFF_T_DECLARED
#define __off_t_defined
#endif

#ifndef __SIZE_TYPE__
#define __SIZE_TYPE__ unsigned long
#endif

#if !defined(_SSIZE_T_DECLARED) && !defined(__ssize_t_defined)
/** @brief Signed size type.  @ingroup posix_option_asynchronous_io*/
#define unsigned signed /* parasoft-suppress MISRAC2012-RULE_20_4-a MISRAC2012-RULE_20_4-b */
typedef __SIZE_TYPE__ ssize_t;
#undef unsigned
#define _SSIZE_T_DECLARED
#define __ssize_t_defined
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

/** @brief Asynchronous I/O control block. */
struct aiocb {
	int aio_fildes;           /**< File descriptor. */
	off_t aio_offset;         /**< File offset for the operation. */
	volatile void *aio_buf;   /**< Data buffer for read or write. */
	size_t aio_nbytes;        /**< Number of bytes to transfer. */
	int aio_reqprio;          /**< Request priority offset. */
#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)
	struct sigevent aio_sigevent; /**< Notification method on completion. */
#endif
	int aio_lio_opcode;       /**< Operation code for lio_listio() (LIO_READ, LIO_WRITE, LIO_NOP). */
};

#if _POSIX_C_SOURCE >= 200112L

/**
 * @brief Cancel an outstanding asynchronous I/O request.
 * @ingroup posix_option_asynchronous_io
 * @param fildes File descriptor.
 * @param aiocbp Control block to cancel, or NULL to cancel all for @p fildes.
 * @return AIO_CANCELED, AIO_NOTCANCELED, AIO_ALLDONE, or -1 on error.
 */
int aio_cancel(int fildes, struct aiocb *aiocbp);

/**
 * @brief Retrieve the error status of an asynchronous I/O request.
 * @ingroup posix_option_asynchronous_io
 * @param aiocbp Asynchronous I/O control block.
 * @return EINPROGRESS if still running, 0 on success, or a positive error number.
 */
int aio_error(const struct aiocb *aiocbp);

/**
 * @brief Asynchronously synchronise a file's data and metadata to storage.
 * @ingroup posix_option_asynchronous_io
 * @param filedes File descriptor (ignored; use aiocbp->aio_fildes).
 * @param aiocbp  Control block specifying the file descriptor.
 * @return 0 if the request was successfully queued, or -1 on failure.
 */
int aio_fsync(int filedes, struct aiocb *aiocbp);

/**
 * @brief Enqueue an asynchronous read operation.
 * @ingroup posix_option_asynchronous_io
 * @param aiocbp Control block specifying the file, offset, buffer, and size.
 * @return 0 if the request was successfully queued, or -1 on failure.
 */
int aio_read(struct aiocb *aiocbp);

/**
 * @brief Retrieve the return status of a completed asynchronous I/O request.
 * @ingroup posix_option_asynchronous_io
 *
 * Must be called exactly once after aio_error() returns a value other than
 * EINPROGRESS.  Calling it a second time yields undefined behaviour.
 *
 * @param aiocbp Asynchronous I/O control block.
 * @return Number of bytes transferred on success, or -1 on error (sets errno).
 */
ssize_t aio_return(struct aiocb *aiocbp);

/**
 * @brief Wait for one or more asynchronous I/O requests to complete.
 * @ingroup posix_option_asynchronous_io
 * @param list    Array of pointers to control blocks to wait on.
 * @param nent    Number of entries in @p list.
 * @param timeout Maximum wait time, or NULL to block indefinitely.
 * @return 0 on success, or -1 with errno set on failure.
 */
int aio_suspend(const struct aiocb *const list[], int nent, const struct timespec *timeout);

/**
 * @brief Enqueue an asynchronous write operation.
 * @ingroup posix_option_asynchronous_io
 * @param aiocbp Control block specifying the file, offset, buffer, and size.
 * @return 0 if the request was successfully queued, or -1 on failure.
 */
int aio_write(struct aiocb *aiocbp);

/**
 * @brief Initiate a list of asynchronous I/O requests.
 * @ingroup posix_option_asynchronous_io
 * @param mode LIO_WAIT (block until all complete) or LIO_NOWAIT (return immediately).
 * @param list Array of control block pointers (LIO_NOP entries are skipped).
 * @param nent Number of entries in @p list.
 * @param sig  Notification on completion (only for LIO_NOWAIT), or NULL.
 * @return 0 on success, or -1 with errno set on failure.
 */
int lio_listio(int mode, struct aiocb *const ZRESTRICT list[], int nent,
	       struct sigevent *ZRESTRICT sig);

#endif /* _POSIX_C_SOURCE >= 200112L */


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_ZEPHYR_POSIX_AIO_H_ */
