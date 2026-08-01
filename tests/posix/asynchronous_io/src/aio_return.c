/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asynchronous_io_tests.h"

static void aio_return_reports_count(void)
{
	char buf[16];
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
		.aio_offset = 0,
	};

	zassert_ok(aio_read(&cb));
	zassert_ok(aio_test_await(&cb));
	zassert_equal(aio_return(&cb), sizeof(buf));
}

static void aio_return_reports_failure(void)
{
	int err;
	struct aiocb cb = {
		.aio_fildes = aio_filero_fd,
		.aio_buf = "nope",
		.aio_nbytes = 4,
	};

	zassert_ok(aio_write(&cb));
	err = aio_test_await(&cb);
	zassert_true(err > 0);

	errno = 0;
	zassert_equal(aio_return(&cb), -1);
	if (!IS_ENABLED(CONFIG_POSIX_TEST_LINUX_COMPAT)) {
		/* POSIX does not require aio_return() to set errno; this implementation does */
		zassert_equal(errno, err);
	}
}

static void aio_return_retrieves_once(void)
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
	zassert_equal(aio_return(&cb), sizeof(buf));

	/* POSIX leaves a second retrieval undefined; this implementation reports EINVAL */
	zassert_equal(aio_return(&cb), -1);
	zassert_equal(errno, EINVAL);
	zassert_equal(aio_error(&cb), -1);
	zassert_equal(errno, EINVAL);

#ifndef CONFIG_NATIVE_LIBC
	zassert_equal(aio_return(NULL), -1);
	zassert_equal(errno, EINVAL);
#endif /* CONFIG_NATIVE_LIBC */
}

ZTEST_USER(posix_asynchronous_io, test_aio_return)
{
	aio_return_reports_count();
	aio_return_reports_failure();
	if (!IS_ENABLED(CONFIG_POSIX_TEST_LINUX_COMPAT)) {
		/* undefined behaviour territory: only asserted against this implementation */
		aio_return_retrieves_once();
	}
}
