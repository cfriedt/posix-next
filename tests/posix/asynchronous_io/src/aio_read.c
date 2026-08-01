/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asynchronous_io_tests.h"

static void aio_read_at_offset(void)
{
	char buf[8] = {0};
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
		.aio_offset = 4,
	};

	zassert_ok(aio_read(&cb));
	zassert_equal(aio_test_reap_ok(&cb), sizeof(buf));
	zassert_mem_equal(buf, &AIO_TEST_CONTENT[4], sizeof(buf));
}

static void aio_read_at_eof(void)
{
	char buf[8];
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
		.aio_offset = AIO_TEST_CONTENT_LEN,
	};

	zassert_ok(aio_read(&cb));
	zassert_equal(aio_test_reap_ok(&cb), 0);
}

static void aio_read_errors(void)
{
	char buf[8];
	int ret;
	struct aiocb cb = {
		.aio_fildes = -1,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
	};

	/* POSIX permits reporting a bad descriptor at submission or completion */
	ret = aio_read(&cb);
	if (ret == 0) {
		zassert_equal(aio_test_await(&cb), EBADF);
		zassert_equal(aio_return(&cb), -1);
	} else {
		zassert_equal(ret, -1);
		zassert_equal(errno, EBADF);
	}

	/* likewise for an invalid offset */
	cb.aio_fildes = aio_file_fd;
	cb.aio_offset = -1;
	ret = aio_read(&cb);
	if (ret == 0) {
		zassert_equal(aio_test_await(&cb), EINVAL);
		zassert_equal(aio_return(&cb), -1);
	} else {
		zassert_equal(ret, -1);
		zassert_equal(errno, EINVAL);
	}

#ifndef CONFIG_NATIVE_LIBC
	zassert_equal(aio_read(NULL), -1);
	zassert_equal(errno, EINVAL);

	/* a priority reduction deeper than AIO_PRIO_DELTA_MAX is rejected */
	cb.aio_offset = 0;
	cb.aio_reqprio = AIO_PRIO_DELTA_MAX + 1;
	zassert_equal(aio_read(&cb), -1);
	zassert_equal(errno, EINVAL);

	/* the deepest expressible reduction is accepted */
	if (AIO_PRIO_DELTA_MAX > 0) {
		cb.aio_reqprio = AIO_PRIO_DELTA_MAX;
		zassert_ok(aio_read(&cb));
		zassert_ok(aio_test_await(&cb));
		zassert_equal(aio_return(&cb), sizeof(buf));
	}
	cb.aio_reqprio = 0;

	/* an unknown notification type is rejected at submission */
	cb.aio_sigevent.sigev_notify = 99;
	zassert_equal(aio_read(&cb), -1);
	zassert_equal(errno, EINVAL);

	/* as is a SIGEV_SIGNAL number outside the supported range */
	cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	cb.aio_sigevent.sigev_signo = 4096;
	zassert_equal(aio_read(&cb), -1);
	zassert_equal(errno, EINVAL);
#endif /* CONFIG_NATIVE_LIBC */
}

static void aio_read_notify_signal(void)
{
	char buf[8] = {0};
	siginfo_t info = {0};
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
		.aio_offset = 0,
		.aio_sigevent = {
			.sigev_notify = SIGEV_SIGNAL,
			.sigev_signo = SIGUSR1,
			.sigev_value.sival_int = 42,
		},
	};

	aio_arm_sig(SIGUSR1);

	zassert_ok(aio_read(&cb));
	/* accept before reap: host AIO may deliver SIGUSR1 as soon as it completes */
	zassert_equal(aio_accept_sig(SIGUSR1, &info, AIO_TEST_TIMEOUT_MS), SIGUSR1);
	zassert_equal(info.si_value.sival_int, 42);
	zassert_equal(aio_test_reap_ok(&cb), sizeof(buf));

#ifndef CONFIG_NATIVE_LIBC
	/* SIGEV_SIGNAL with signal number 0 completes without notifying */
	cb.aio_sigevent.sigev_signo = 0;
	zassert_ok(aio_read(&cb));
	zassert_equal(aio_test_reap_ok(&cb), sizeof(buf));
	zassert_equal(aio_accept_sig(SIGUSR1, NULL, 100), -1);
#endif /* CONFIG_NATIVE_LIBC */
}

ZTEST_USER(posix_asynchronous_io, test_aio_read)
{
	aio_read_at_offset();
	aio_read_at_eof();
	aio_read_errors();
	aio_read_notify_signal();
}
