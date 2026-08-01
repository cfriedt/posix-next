/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asynchronous_io_tests.h"

static void lio_listio_wait(void)
{
	char buf[8] = {0};
	struct aiocb rcb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
		.aio_offset = 0,
		.aio_lio_opcode = LIO_READ,
	};
	struct aiocb wcb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = "YYYY",
		.aio_nbytes = 4,
		.aio_offset = 8,
		.aio_lio_opcode = LIO_WRITE,
	};
	struct aiocb nop = {
		.aio_lio_opcode = LIO_NOP,
	};
	struct aiocb *const list[] = {&rcb, NULL, &nop, &wcb};

	zassert_ok(lio_listio(LIO_WAIT, list, ARRAY_SIZE(list), NULL));

	/* every operation has completed on return */
	zassert_ok(aio_error(&rcb));
	zassert_ok(aio_error(&wcb));
	zassert_equal(aio_return(&rcb), sizeof(buf));
	zassert_equal(aio_return(&wcb), 4);
	zassert_mem_equal(buf, AIO_TEST_CONTENT, sizeof(buf));
}

static void lio_listio_nowait(void)
{
	char buf[8] = {0};
	struct aiocb rcb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = buf,
		.aio_nbytes = sizeof(buf),
		.aio_offset = 0,
		.aio_lio_opcode = LIO_READ,
	};
	struct aiocb *const list[] = {&rcb};

	zassert_ok(lio_listio(LIO_NOWAIT, list, ARRAY_SIZE(list), NULL));
	zassert_equal(aio_test_reap_ok(&rcb), sizeof(buf));
	zassert_mem_equal(buf, AIO_TEST_CONTENT, sizeof(buf));
}

static void lio_listio_nowait_signal(void)
{
	char b1[4];
	char b2[4];
	siginfo_t info;
	struct aiocb cb1 = {
		.aio_fildes = aio_file_fd,
		.aio_buf = b1,
		.aio_nbytes = sizeof(b1),
		.aio_offset = 0,
		.aio_lio_opcode = LIO_READ,
	};
	struct aiocb cb2 = {
		.aio_fildes = aio_file_fd,
		.aio_buf = b2,
		.aio_nbytes = sizeof(b2),
		.aio_offset = 4,
		.aio_lio_opcode = LIO_READ,
	};
	struct aiocb *const list[] = {&cb1, &cb2};
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGUSR1,
		.sigev_value.sival_int = 99,
	};

	aio_arm_sig(SIGUSR1);

	zassert_ok(lio_listio(LIO_NOWAIT, list, ARRAY_SIZE(list), &sig));

	/* one notification once the whole list has completed */
	zassert_equal(aio_accept_sig(SIGUSR1, &info, AIO_TEST_TIMEOUT_MS), SIGUSR1);
	zassert_equal(info.si_code, SI_ASYNCIO);
	zassert_equal(info.si_value.sival_int, 99);

	zassert_equal(aio_test_reap_ok(&cb1), sizeof(b1));
	zassert_equal(aio_test_reap_ok(&cb2), sizeof(b2));
}

static void lio_listio_wait_failure(void)
{
	struct aiocb bad = {
		.aio_fildes = aio_filero_fd,
		.aio_buf = "nope",
		.aio_nbytes = 4,
		.aio_lio_opcode = LIO_WRITE,
	};
	struct aiocb *const list[] = {&bad};

	/* one of the operations fails: EIO, with per-operation status retrievable */
	zassert_equal(lio_listio(LIO_WAIT, list, ARRAY_SIZE(list), NULL), -1);
	zassert_equal(errno, EIO);
	zassert_equal(aio_return(&bad), -1);

	/* an unknown opcode fails that entry, and the awaited list with it */
	bad.aio_fildes = aio_file_fd;
	bad.aio_lio_opcode = 0x5a5a;
	zassert_equal(lio_listio(LIO_WAIT, list, ARRAY_SIZE(list), NULL), -1);
	zassert_equal(errno, EIO);

	/* under LIO_NOWAIT it fails only that entry, reported through its status */
	zassert_ok(lio_listio(LIO_NOWAIT, list, ARRAY_SIZE(list), NULL));
	zassert_equal(aio_test_await(&bad), EINVAL);
	zassert_equal(aio_cancel(aio_file_fd, &bad), AIO_ALLDONE);
	zassert_equal(aio_return(&bad), -1);

#ifndef CONFIG_NATIVE_LIBC
	/* a submission this implementation rejects up front fails the list call */
	bad.aio_lio_opcode = LIO_READ;
	bad.aio_offset = -1;
	zassert_equal(lio_listio(LIO_NOWAIT, list, ARRAY_SIZE(list), NULL), -1);
	zassert_equal(errno, EIO);
#endif /* CONFIG_NATIVE_LIBC */
}

static void lio_listio_errors(void)
{
	struct aiocb *const list[] = {NULL};

	zassert_equal(lio_listio(0x5a5a, list, 1, NULL), -1);
	zassert_equal(errno, EINVAL);

#ifndef CONFIG_NATIVE_LIBC
	/* an invalid list notification is rejected before anything submits */
	struct sigevent sig = {
		.sigev_notify = SIGEV_THREAD,
		.sigev_notify_function = NULL,
	};

	zassert_equal(lio_listio(LIO_NOWAIT, list, 1, &sig), -1);
	zassert_equal(errno, EINVAL);
#endif /* CONFIG_NATIVE_LIBC */
}

#ifndef CONFIG_NATIVE_LIBC
#define GROUP_EXHAUSTED_N 4

/* wait until at least @p n of @p cbs have left EINPROGRESS */
static void await_completed(const struct aiocb *cbs, size_t n)
{
	const struct timespec delay = {
		.tv_nsec = 10000000L,
	};

	for (int t = 0; t < AIO_TEST_TIMEOUT_MS / 10; t++) {
		const struct aiocb *pending[GROUP_EXHAUSTED_N];
		size_t npending = 0;

		for (size_t i = 0; i < GROUP_EXHAUSTED_N; i++) {
			if (aio_error(&cbs[i]) == EINPROGRESS) {
				pending[npending++] = &cbs[i];
			}
		}

		if (GROUP_EXHAUSTED_N - npending >= n) {
			return;
		}

		(void)aio_suspend(pending, npending, &delay);
	}

	zassert_true(false, "fewer than %zu of %d reads completed", n, GROUP_EXHAUSTED_N);
}

/* only meaningful when the request pool cannot grow (the static_only variant) */
static void lio_listio_group_exhausted(void)
{
	eventfd_t values[GROUP_EXHAUSTED_N];
	struct aiocb cbs[GROUP_EXHAUSTED_N];
	struct aiocb nop = {
		.aio_lio_opcode = LIO_NOP,
	};
	struct aiocb *const list[] = {&nop};
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGUSR1,
	};

	for (size_t i = 0; i < ARRAY_SIZE(cbs); i++) {
		cbs[i] = (struct aiocb){
			.aio_fildes = aio_efd,
			.aio_buf = &values[i],
			.aio_nbytes = sizeof(values[i]),
		};
		zassert_ok(aio_read(&cbs[i]));
	}

	/* the completion group needs a pool slot too */
	zassert_equal(lio_listio(LIO_NOWAIT, list, ARRAY_SIZE(list), &sig), -1);
	zassert_equal(errno, EAGAIN);

	/* each value releases exactly one read, but which one is unspecified */
	for (size_t i = 0; i < ARRAY_SIZE(cbs); i++) {
		aio_test_efd_add(aio_efd, 1);
		await_completed(cbs, i + 1);
	}

	for (size_t i = 0; i < ARRAY_SIZE(cbs); i++) {
		zassert_equal(aio_test_reap_ok(&cbs[i]), sizeof(values[i]));
	}
}
#endif /* CONFIG_NATIVE_LIBC */

ZTEST_USER(posix_asynchronous_io, test_lio_listio)
{
	lio_listio_wait();
	lio_listio_nowait();
	lio_listio_nowait_signal();
	lio_listio_wait_failure();
	lio_listio_errors();
#ifndef CONFIG_NATIVE_LIBC
	if (CONFIG_SYS_AIO_MAX == 4) {
		lio_listio_group_exhausted();
	}
#endif
}
