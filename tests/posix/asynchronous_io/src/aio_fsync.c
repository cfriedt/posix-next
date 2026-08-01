/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asynchronous_io_tests.h"

static void aio_fsync_sync(int op)
{
	struct aiocb wcb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = "sync",
		.aio_nbytes = 4,
		.aio_offset = 0,
	};
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
	};

	zassert_ok(aio_write(&wcb));
	zassert_equal(aio_test_reap_ok(&wcb), 4);

	zassert_ok(aio_fsync(op, &cb));
	zassert_equal(aio_test_reap_ok(&cb), 0);
}

static void aio_fsync_errors(void)
{
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
	};

	zassert_equal(aio_fsync(0x5a5a, &cb), -1);
	zassert_equal(errno, EINVAL);
}

ZTEST_USER(posix_asynchronous_io, test_aio_fsync)
{
	aio_fsync_sync(O_SYNC);
	aio_fsync_sync(O_DSYNC);
	aio_fsync_errors();
}
