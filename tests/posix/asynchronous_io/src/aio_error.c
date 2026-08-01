/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asynchronous_io_tests.h"

static void aio_error_tracks_completion(void)
{
	eventfd_t value = 0;
	struct aiocb cb = {
		.aio_fildes = aio_efd,
		.aio_buf = &value,
		.aio_nbytes = sizeof(value),
	};

	zassert_ok(aio_read(&cb));

	/* nothing to read yet: the operation is outstanding */
	zassert_equal(aio_error(&cb), EINPROGRESS);

	aio_test_efd_add(aio_efd, 7);
	zassert_ok(aio_test_await(&cb));

	/* status polling is idempotent until retrieved */
	zassert_ok(aio_error(&cb));
	zassert_equal(aio_return(&cb), sizeof(value));
	zassert_equal(value, 7);
}

static void aio_error_reports_failure(void)
{
	int err;
	struct aiocb cb = {
		.aio_fildes = aio_filero_fd,
		.aio_buf = "nope",
		.aio_nbytes = 4,
	};

	zassert_ok(aio_write(&cb));
	err = aio_test_await(&cb);
	zassert_true((err == EBADF) || (err == EACCES), "unexpected status %d", err);
	zassert_equal(aio_return(&cb), -1);

#ifndef CONFIG_NATIVE_LIBC
	zassert_equal(aio_error(NULL), -1);
	zassert_equal(errno, EINVAL);
#endif /* CONFIG_NATIVE_LIBC */
}

ZTEST_USER(posix_asynchronous_io, test_aio_error)
{
	aio_error_tracks_completion();
	aio_error_reports_failure();
}
