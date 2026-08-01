/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asynchronous_io_tests.h"

static void aio_suspend_completed_returns_immediately(void)
{
	char buf[4];
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
		.aio_offset = 0,
	};
	const struct aiocb *const list[] = {&cb};
	const struct timespec zero = {0};

	zassert_ok(aio_read(&cb));
	zassert_ok(aio_test_await(&cb));

	/* an already-completed operation never waits, even with a zero timeout */
	zassert_ok(aio_suspend(list, 1, &zero));
	zassert_equal(aio_return(&cb), sizeof(buf));
}

static void aio_suspend_times_out(void)
{
	eventfd_t value;
	struct aiocb cb = {
		.aio_fildes = aio_efd,
		.aio_buf = &value,
		.aio_nbytes = sizeof(value),
	};
	const struct aiocb *const list[] = {&cb};
	const struct timespec short_to = {
		.tv_nsec = 50000000L,
	};

	zassert_ok(aio_read(&cb));

	zassert_equal(aio_suspend(list, 1, &short_to), -1);
	zassert_equal(errno, EAGAIN);

	/* completing the operation satisfies the wait */
	aio_test_efd_add(aio_efd, 1);
	zassert_ok(aio_suspend(list, 1, NULL));
	zassert_equal(aio_test_reap_ok(&cb), sizeof(value));
}

static void aio_suspend_any_of_list(void)
{
	char buf[4];
	eventfd_t value;
	struct aiocb pending = {
		.aio_fildes = aio_efd,
		.aio_buf = &value,
		.aio_nbytes = sizeof(value),
	};
	struct aiocb quick = {
		.aio_fildes = aio_file_fd,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
		.aio_offset = 0,
	};
	const struct aiocb *const list[] = {&pending, NULL, &quick};

	zassert_ok(aio_read(&pending));
	zassert_ok(aio_read(&quick));

	/* the file read completes; the eventfd read stays outstanding */
	zassert_ok(aio_suspend(list, ARRAY_SIZE(list), NULL));
	zassert_equal(aio_test_reap_ok(&quick), sizeof(buf));

	aio_test_efd_add(aio_efd, 1);
	zassert_equal(aio_test_reap_ok(&pending), sizeof(value));
}

ZTEST_USER(posix_asynchronous_io, test_aio_suspend)
{
	aio_suspend_completed_returns_immediately();
	aio_suspend_times_out();
	aio_suspend_any_of_list();
}
