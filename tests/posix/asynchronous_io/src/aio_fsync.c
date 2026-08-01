/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

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

#ifndef CONFIG_NATIVE_LIBC
	zassert_equal(aio_fsync(O_SYNC, NULL), -1);
	zassert_equal(errno, EINVAL);
#endif /* CONFIG_NATIVE_LIBC */
}

#ifndef CONFIG_NATIVE_LIBC
/* SIGEV_THREAD_ID needs a pthread handle; the host libc wants a kernel tid */
static void aio_fsync_notify_thread_id(void)
{
	siginfo_t info = {0};
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
		.aio_sigevent = {
			.sigev_notify = SIGEV_THREAD_ID,
			.sigev_signo = SIGUSR2,
			.sigev_value.sival_int = 17,
			.sigev_notify_thread_id = (pid_t)pthread_self(),
		},
	};

	aio_arm_sig(SIGUSR2);

	/* completion signals the designated thread */
	zassert_ok(aio_fsync(O_SYNC, &cb));
	zassert_equal(aio_test_reap_ok(&cb), 0);
	zassert_equal(aio_accept_sig(SIGUSR2, &info, AIO_TEST_TIMEOUT_MS), SIGUSR2);
	zassert_equal(info.si_value.sival_int, 17);

	/* an out-of-range signal is rejected at submission */
	cb.aio_sigevent.sigev_signo = 4096;
	zassert_equal(aio_fsync(O_SYNC, &cb), -1);
	zassert_equal(errno, EINVAL);
}
#endif /* CONFIG_NATIVE_LIBC */

ZTEST_USER(posix_asynchronous_io, test_aio_fsync)
{
	aio_fsync_sync(O_SYNC);
	aio_fsync_sync(O_DSYNC);
	aio_fsync_errors();
#ifndef CONFIG_NATIVE_LIBC
	aio_fsync_notify_thread_id();
#endif
}
