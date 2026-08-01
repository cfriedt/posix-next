/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

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

#ifndef CONFIG_NATIVE_LIBC
	zassert_equal(aio_write(NULL), -1);
	zassert_equal(errno, EINVAL);

	/* SIGEV_THREAD without a notification function is rejected */
	cb.aio_fildes = aio_file_fd;
	cb.aio_sigevent.sigev_notify = SIGEV_THREAD;
	cb.aio_sigevent.sigev_notify_function = NULL;
	zassert_equal(aio_write(&cb), -1);
	zassert_equal(errno, EINVAL);
#endif /* CONFIG_NATIVE_LIBC */
}

static ZTEST_BMEM volatile int notify_thread_hits;

static void aio_write_notify_fn(union sigval value)
{
	zassert_equal(value.sival_int, 7);
	notify_thread_hits++;
}

static void aio_write_notify_await(int hits)
{
	const struct timespec delay = {
		.tv_nsec = 10000000L,
	};

	for (int i = 0; (i < (AIO_TEST_TIMEOUT_MS / 10)) && (notify_thread_hits < hits); i++) {
		(void)nanosleep(&delay, NULL);
	}
	zassert_equal(notify_thread_hits, hits, "notification function did not run");
}

static void aio_write_notify_thread(void)
{
	pthread_attr_t attr;
	struct aiocb cb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = "note",
		.aio_nbytes = 4,
		.aio_offset = 0,
		.aio_sigevent = {
			.sigev_notify = SIGEV_THREAD,
			.sigev_notify_function = aio_write_notify_fn,
			.sigev_value.sival_int = 7,
		},
	};

	notify_thread_hits = 0;

	/* completion runs the notification function in a fresh thread */
	zassert_ok(aio_write(&cb));
	zassert_equal(aio_test_reap_ok(&cb), 4);
	aio_write_notify_await(1);

	/* thread attributes are honored at submission time */
	zassert_ok(pthread_attr_init(&attr));
	zassert_ok(pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN));
	cb.aio_sigevent.sigev_notify_attributes = &attr;
	zassert_ok(aio_write(&cb));
	zassert_equal(aio_test_reap_ok(&cb), 4);
	aio_write_notify_await(2);
	zassert_ok(pthread_attr_destroy(&attr));
}

#if defined(CONFIG_POSIX_PRIORITIZED_IO) && !defined(CONFIG_NATIVE_LIBC)
static void aio_write_prioritized(void)
{
	char check[4] = {0};
	struct aiocb low = {
		.aio_fildes = aio_file_fd,
		.aio_buf = "loww",
		.aio_nbytes = 4,
		.aio_offset = 0,
		.aio_reqprio = AIO_PRIO_DELTA_MAX,
	};
	struct aiocb high = {
		.aio_fildes = aio_file_fd,
		.aio_buf = "high",
		.aio_nbytes = 4,
		.aio_offset = 0,
	};
	struct aiocb rcb = {
		.aio_fildes = aio_file_fd,
		.aio_buf = check,
		.aio_nbytes = sizeof(check),
		.aio_offset = 0,
	};

	/*
	 * With the scheduler locked both writes are queued before the service
	 * thread may run: the higher-priority write to the same offset
	 * executes first, so the lower-priority write's bytes land last.
	 */
	k_sched_lock();
	zassert_ok(aio_write(&low));
	zassert_ok(aio_write(&high));
	k_sched_unlock();

	zassert_equal(aio_test_reap_ok(&low), 4);
	zassert_equal(aio_test_reap_ok(&high), 4);

	zassert_ok(aio_read(&rcb));
	zassert_equal(aio_test_reap_ok(&rcb), sizeof(check));
	zassert_mem_equal(check, "loww", 4);
}
#endif /* CONFIG_POSIX_PRIORITIZED_IO && !CONFIG_NATIVE_LIBC */

ZTEST_USER(posix_asynchronous_io, test_aio_write)
{
	aio_write_at_offset();
	aio_write_extends_file();
	aio_write_errors();
	aio_write_notify_thread();
#if defined(CONFIG_POSIX_PRIORITIZED_IO) && !defined(CONFIG_NATIVE_LIBC)
	if (!IS_ENABLED(CONFIG_SMP) && !k_is_user_context()) {
		aio_write_prioritized();
	}
#endif
}
