/*
 * Copyright (c) 2020 Tobias Svehagen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "_main.h"

ZTEST_F(eventfd, test_write_then_read)
{
	eventfd_t val;
	int ret;

	ret = eventfd_write(fixture->fd, 3);
	zassert_true(ret == 0, "write ret %d", ret);

	ret = eventfd_write(fixture->fd, 2);
	zassert_true(ret == 0, "write ret %d", ret);

	ret = eventfd_read(fixture->fd, &val);
	zassert_true(ret == 0, "read ret %d", ret);
	zassert_true(val == 5, "val == %lld", val);

	/* Test EFD_SEMAPHORE */
	reopen(&fixture->fd, 0, EFD_SEMAPHORE);

	ret = eventfd_write(fixture->fd, 3);
	zassert_true(ret == 0, "write ret %d", ret);

	ret = eventfd_write(fixture->fd, 2);
	zassert_true(ret == 0, "write ret %d", ret);

	ret = eventfd_read(fixture->fd, &val);
	zassert_true(ret == 0, "read ret %d", ret);
	zassert_true(val == 1, "val == %lld", val);
}

ZTEST_F(eventfd, test_zero_shall_not_unblock)
{
	short event;
	int ret;

	ret = eventfd_write(fixture->fd, 0);
	zassert_equal(ret, 0, "fd == %d", fixture->fd);

	event = POLLIN;
	ret = is_blocked(fixture->fd, &event);
	zassert_equal(ret, 1, "eventfd unblocked by zero");
}

ZTEST_F(eventfd, test_poll_timeout)
{
	struct pollfd pfd;
	int ret;

	pfd.fd = fixture->fd;
	pfd.events = POLLIN;

	ret = poll(&pfd, 1, 500);
	zassert_true(ret == 0, "poll ret %d", ret);
}

ZTEST_F(eventfd, test_set_poll_event_block)
{
	reopen(&fixture->fd, TESTVAL, 0);
	eventfd_poll_set_common(fixture->fd);
}

ZTEST_F(eventfd, test_unset_poll_event_block)
{
	eventfd_poll_unset_common(fixture->fd);
}

static void *thread_eventfd_read_42(void *arg1)
{
	eventfd_t value;
	struct eventfd_fixture *fixture = arg1;

	zassert_ok(eventfd_read(fixture->fd, &value));
	zassert_equal(value, 42);

	return NULL;
}

ZTEST_F(eventfd, test_read_then_write_block)
{
	pthread_t th;

	zassert_ok(pthread_create(&th, NULL, thread_eventfd_read_42, fixture));

	usleep(100000);

	/* this write never occurs */
	zassert_ok(eventfd_write(fixture->fd, 42));

	/* unreachable code */
	zassert_ok(pthread_join(th, NULL));
}

static void *thread_eventfd_posix_read_42(void *arg1)
{
	uint64_t value;
	struct eventfd_fixture *fixture = arg1;
	int ret;

	ret = read(fixture->fd, &value, sizeof(value));
	zassert(ret == sizeof(value), "read(2) failed");
	zassert_equal(value, 42);

	return NULL;
}

ZTEST_F(eventfd, test_posix_read_then_write_block)
{
	pthread_t th;

	zassert_ok(pthread_create(&th, NULL, thread_eventfd_posix_read_42, fixture));

	usleep(100000);

	zassert_ok(eventfd_write(fixture->fd, 42));

	zassert_ok(pthread_join(th, NULL));
}

static void *thread_eventfd_write(void *arg1)
{
	struct eventfd_fixture *fixture = arg1;

	zassert_ok(eventfd_write(fixture->fd, 71));

	return NULL;
}

ZTEST_F(eventfd, test_write_while_pollin)
{
	struct pollfd fds[] = {
		{
			.fd = fixture->fd,
			.events = POLLIN,
		},
	};
	eventfd_t value;
	pthread_t th;
	int ret;

	zassert_ok(pthread_create(&th, NULL, thread_eventfd_write, fixture));

	/* Expect 1 event */
	ret = poll(fds, ARRAY_SIZE(fds), 200);
	zassert_equal(ret, 1);

	zassert_equal(fds[0].revents, POLLIN);

	/* Check value */
	zassert_ok(eventfd_read(fixture->fd, &value));
	zassert_equal(value, 71);

	zassert_ok(pthread_join(th, NULL));
}

static void *thread_eventfd_read(void *arg1)
{
	eventfd_t value;
	struct eventfd_fixture *fixture = arg1;

	usleep(100000);

	zassert_ok(eventfd_read(fixture->fd, &value));

	return NULL;
}

ZTEST_F(eventfd, test_read_while_pollout)
{
	struct pollfd fds[] = {
		{
			.fd = fixture->fd,
			.events = POLLOUT,
		},
	};
	pthread_t th;
	int ret;

	zassert_ok(eventfd_write(fixture->fd, UINT64_MAX - 1));

	zassert_ok(pthread_create(&th, NULL, thread_eventfd_read, fixture));

	/* Expect 1 event */
	ret = poll(fds, ARRAY_SIZE(fds), 200);
	zassert_equal(ret, 1);

	zassert_equal(fds[0].revents, POLLOUT);

	zassert_ok(pthread_join(th, NULL));
}
