/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asynchronous_io_tests.h"

static void aio_write_at_offset(void)
{
	char check[AIO_TEST_CONTENT_LEN] = {0};
	struct aiocb wcb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = "XXXX",
		.aio_nbytes = 4,
		.aio_offset = 4,
	};
	struct aiocb rcb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = check,
		.aio_nbytes = sizeof(check),
		.aio_offset = 0,
	};

	zassert_ok(aio_write(&wcb));
	zassert_equal(aio_test_reap_ok(&wcb), 4);

	zassert_ok(aio_read(&rcb));
	zassert_equal(aio_test_reap_ok(&rcb), sizeof(check));
	zassert_mem_equal(check, AIO_TEST_CONTENT, 4);
	zassert_mem_equal(&check[4], "XXXX", 4);
	zassert_mem_equal(&check[8], &AIO_TEST_CONTENT[8], AIO_TEST_CONTENT_LEN - 8);
}

static void aio_write_extends_file(void)
{
	char check[4] = {0};
	struct aiocb wcb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = "tail",
		.aio_nbytes = 4,
		.aio_offset = AIO_TEST_CONTENT_LEN,
	};
	struct aiocb rcb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = check,
		.aio_nbytes = sizeof(check),
		.aio_offset = AIO_TEST_CONTENT_LEN,
	};

	zassert_ok(aio_write(&wcb));
	zassert_equal(aio_test_reap_ok(&wcb), 4);

	zassert_ok(aio_read(&rcb));
	zassert_equal(aio_test_reap_ok(&rcb), sizeof(check));
	zassert_mem_equal(check, "tail", 4);
}

static void aio_write_errors(void)
{
	int err;
	struct aiocb cb = {
		.aio_fildes = aio_filero_fd,
		.aio_buf = "nope",
		.aio_nbytes = 4,
	};

	/* writing through a read-only descriptor fails at execution time */
	zassert_ok(aio_write(&cb));
	err = aio_test_await(&cb);
	zassert_true((err == EBADF) || (err == EACCES), "unexpected status %d", err);
	zassert_equal(aio_return(&cb), -1);
}

ZTEST_USER(posix_asynchronous_io, test_aio_write)
{
	aio_write_at_offset();
	aio_write_extends_file();
	aio_write_errors();
}
