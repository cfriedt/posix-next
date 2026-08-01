/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asynchronous_io_tests.h"

static void aio_cancel_completed(void)
{
	char buf[4];
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
		.aio_offset = 0,
	};

	zassert_ok(aio_read(&cb));
	zassert_ok(aio_test_await(&cb));

	zassert_equal(aio_cancel(aio_file_fd, &cb), AIO_ALLDONE);
	zassert_equal(aio_return(&cb), sizeof(buf));
}

static void aio_cancel_outstanding(void)
{
	eventfd_t value;
	int ret;
	struct aiocb cb = {
		.aio_fildes = aio_efd,
		.aio_buf = &value,
		.aio_nbytes = sizeof(value),
	};

	zassert_ok(aio_read(&cb));

	/* an implementation may already be executing the request */
	ret = aio_cancel(aio_efd, &cb);
	zassert_true((ret == AIO_CANCELED) || (ret == AIO_NOTCANCELED), "unexpected %d", ret);

	if (ret == AIO_CANCELED) {
		zassert_equal(aio_test_await(&cb), ECANCELED);
		zassert_equal(aio_return(&cb), -1);
	} else {
		aio_test_efd_add(aio_efd, 1);
		zassert_equal(aio_test_reap_ok(&cb), sizeof(value));
	}
}

static void aio_cancel_all_idle(void)
{
	/* nothing outstanding on the descriptor */
	zassert_equal(aio_cancel(aio_file_fd, NULL), AIO_ALLDONE);
}

static void aio_cancel_errors(void)
{
	zassert_equal(aio_cancel(-1, NULL), -1);
	zassert_equal(errno, EBADF);
}

ZTEST_USER(posix_asynchronous_io, test_aio_cancel)
{
	aio_cancel_completed();
	aio_cancel_outstanding();
	aio_cancel_all_idle();
	aio_cancel_errors();
}
