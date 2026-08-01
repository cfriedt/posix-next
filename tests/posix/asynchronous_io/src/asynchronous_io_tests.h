/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_ASYNCHRONOUS_IO_TESTS_H_
#define POSIX_ASYNCHRONOUS_IO_TESTS_H_

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <sys/eventfd.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

/* fixture descriptors, reopened before every test */
extern int aio_file_fd;   /* AIO_TEST_FILE, read-write, filled with AIO_TEST_CONTENT */
extern int aio_filero_fd; /* AIO_TEST_FILE, read-only */
extern int aio_efd;       /* eventfd, initially unreadable */

#define AIO_TEST_CONTENT "The quick brown fox jumps over the lazy dog!"
#define AIO_TEST_CONTENT_LEN (sizeof(AIO_TEST_CONTENT) - 1)

#define AIO_TEST_TIMEOUT_MS 5000

/**
 * @brief Accept one instance of @p signo, waiting up to @p timeout_ms.
 *
 * On @kconfig{CONFIG_NATIVE_LIBC} the signal is a real host signal: a blocking
 * host sigtimedwait() would stall native_sim, so deliveries are captured by a
 * handler and consumed from a counter instead. The counter is cleared by
 * aio_arm_sig(), so a delivery that races ahead of this wait is still seen.
 *
 * @return the signal number, or -1 if none arrived in time.
 */
int aio_accept_sig(int signo, siginfo_t *info, int timeout_ms);

/**
 * @brief Arrange for @p signo to be accepted by aio_accept_sig().
 *
 * Under @kconfig{CONFIG_NATIVE_LIBC}, also clears the capture counter so later
 * accepts only see deliveries after this arm.
 */
void aio_arm_sig(int signo);

/**
 * @brief Wait until the operation on @p acb completes.
 *
 * Waits with a bounded aio_suspend() polling loop so it is portable to truly
 * asynchronous implementations (the host libc under linux_compat).
 *
 * @return the completion status, as aio_error().
 */
static inline int aio_test_await(const struct aiocb *acb)
{
	const struct timespec delay = {
		.tv_nsec = 10000000L,
	};

	for (int i = 0; i < (AIO_TEST_TIMEOUT_MS / 10); i++) {
		const int err = aio_error(acb);

		if (err != EINPROGRESS) {
			return err;
		}
		(void)aio_suspend((const struct aiocb *const[]){acb}, 1, &delay);
	}

	return aio_error(acb);
}

/** @brief Await successful completion of @p acb and reap its byte count. */
static inline ssize_t aio_test_reap_ok(struct aiocb *acb)
{
	zassert_ok(aio_test_await(acb), "operation failed");

	return aio_return(acb);
}

/**
 * @brief Make @p fd (an eventfd) readable by adding @p value to its counter.
 *
 * Written with write() rather than eventfd_write(): plain descriptor I/O is
 * usable from user mode and against the host libc alike.
 */
static inline void aio_test_efd_add(int fd, eventfd_t value)
{
	zassert_equal(write(fd, &value, sizeof(value)), (ssize_t)sizeof(value));
}

#endif /* POSIX_ASYNCHRONOUS_IO_TESTS_H_ */
