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
}

ZTEST_USER(posix_asynchronous_io, test_aio_read)
{
	aio_read_at_offset();
	aio_read_at_eof();
	aio_read_errors();
}
