/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fifo_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void mkfifo_node(void)
{
	struct stat st;

	zassert_ok(mkfifo(TEST_FIFO, 0644));
	zassert_ok(stat(TEST_FIFO, &st));
	zassert_true(S_ISFIFO(st.st_mode));
	zassert_equal(st.st_size, 0);

	errno = 0;
	zassert_equal(mkfifo(TEST_FIFO, 0644), -1);
	zassert_equal(errno, EEXIST);
	errno = 0;
	zassert_equal(mkfifo(TEST_FILE, 0644), -1);
	zassert_equal(errno, EEXIST);
	errno = 0;
	zassert_equal(mkfifo(TEST_NOENT "/fifo", 0644), -1);
	zassert_equal(errno, ENOENT);
}

/* the other side of the exchange: opens for writing, sends the message, leaves */
static void *fifo_writer(void *arg)
{
	int fd = open(TEST_FIFO, O_WRONLY);

	ARG_UNUSED(arg);

	if (fd < 0) {
		return (void *)(intptr_t)errno;
	}
	if (write(fd, TEST_MESSAGE, strlen(TEST_MESSAGE)) != (ssize_t)strlen(TEST_MESSAGE)) {
		return (void *)(intptr_t)errno;
	}
	if (close(fd) < 0) {
		return (void *)(intptr_t)errno;
	}

	return NULL;
}

static void mkfifo_exchange(void)
{
	char buf[sizeof(TEST_MESSAGE)];
	pthread_t writer;
	void *status;
	int fd;

	/* the writer blocks in open() until a reader arrives */
	zassert_ok(pthread_create(&writer, NULL, fifo_writer, NULL));
	fd = open(TEST_FIFO, O_RDONLY);
	zassert_true(fd >= 0, "open(fifo) failed: %d", errno);

	zassert_equal(read(fd, buf, sizeof(buf)), strlen(TEST_MESSAGE));
	zassert_mem_equal(buf, TEST_MESSAGE, strlen(TEST_MESSAGE));
	zassert_ok(pthread_join(writer, &status));
	zassert_is_null(status, "writer failed: %d", (int)(intptr_t)status);

	/* the last writer gone and the pipe drained: end-of-file */
	zassert_equal(read(fd, buf, sizeof(buf)), 0);
	zassert_ok(close(fd));
}

static void mkfifo_nonblock(void)
{
	char buf[8];
	int rfd, wfd;

	/* a writer needs a reader; a reader proceeds alone */
	errno = 0;
	zassert_equal(open(TEST_FIFO, O_WRONLY | O_NONBLOCK), -1);
	zassert_equal(errno, ENXIO);

	rfd = open(TEST_FIFO, O_RDONLY | O_NONBLOCK);
	zassert_true(rfd >= 0, "open(fifo, nonblock) failed: %d", errno);
	/* nobody writing: end-of-file, not "try again" */
	zassert_equal(read(rfd, buf, sizeof(buf)), 0);

	wfd = open(TEST_FIFO, O_WRONLY | O_NONBLOCK);
	zassert_true(wfd >= 0, "open(fifo, nonblock) failed: %d", errno);
	/* a writer connected but silent: try again */
	errno = 0;
	zassert_equal(read(rfd, buf, sizeof(buf)), -1);
	zassert_equal(errno, EAGAIN);
	zassert_equal(write(wfd, "ab", 2), 2);
	zassert_equal(read(rfd, buf, sizeof(buf)), 2);
	zassert_mem_equal(buf, "ab", 2);
	zassert_ok(close(wfd));
	zassert_equal(read(rfd, buf, sizeof(buf)), 0);
	zassert_ok(close(rfd));

	/* the exchange can start over once everyone has left */
	rfd = open(TEST_FIFO, O_RDONLY | O_NONBLOCK);
	zassert_true(rfd >= 0);
	wfd = open(TEST_FIFO, O_WRONLY);
	zassert_true(wfd >= 0);
	zassert_ok(close(wfd));
	zassert_ok(close(rfd));

	/* opening for both directions is undefined; refused here, taken by the host */
	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(open(TEST_FIFO, O_RDWR), -1);
		zassert_equal(errno, EINVAL);
	});
}

static void mkfifo_remove(void)
{
	struct stat st;

	zassert_ok(unlink(TEST_FIFO));
	errno = 0;
	zassert_equal(stat(TEST_FIFO, &st), -1);
	zassert_equal(errno, ENOENT);
}

ZTEST_USER(posix_fifo, test_mkfifo)
{
	mkfifo_node();
	mkfifo_exchange();
	mkfifo_nonblock();
	mkfifo_remove();
}
