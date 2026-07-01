/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#include "test_fs.h"

static void cleanup_test_file(void)
{
	(void)unlink(TEST_FILE);
}

ZTEST(posix_device_io, test_read_write)
{
	static const char payload[] = "read-write";
	char buf[sizeof(payload)] = {0};
	int fd;

	cleanup_test_file();

	fd = open(TEST_FILE, O_CREAT | O_RDWR, 0660);
	zassert_true(fd >= 0, "open() failed, errno=%d", errno);

	zassert_equal(lseek(fd, 0, SEEK_SET), 0);
	zassert_equal(write(fd, payload, strlen(payload)), (ssize_t)strlen(payload));
	zassert_equal(lseek(fd, 0, SEEK_SET), 0);
	zassert_equal(read(fd, buf, strlen(payload)), (ssize_t)strlen(payload));
	zassert_mem_equal(buf, payload, strlen(payload), "read() data mismatch");

	zassert_ok(close(fd));
	cleanup_test_file();
}

ZTEST(posix_device_io, test_fdopen_fileno)
{
	FILE *fp;
	int fd;

	cleanup_test_file();

	fd = open(TEST_FILE, O_CREAT | O_RDWR, 0660);
	zassert_true(fd >= 0, "open() failed, errno=%d", errno);

	fp = fdopen(fd, "r+");
	zassert_not_null(fp, "fdopen() failed, errno=%d", errno);
	zassert_equal(fileno(fp), fd, "fileno(fdopen(%d)) != %d", fd, fd);

	zassert_ok(fclose(fp));
	cleanup_test_file();
}

ZTEST(posix_device_io, test_fdopen_fwrite_read)
{
	FILE *fp;
	eventfd_t write_val = 42;
	eventfd_t read_val = 0;
	ssize_t n;
	int efd;

	efd = eventfd(0, 0);
	zassert_true(efd >= 0, "eventfd() failed, errno=%d", errno);

	fp = fdopen(efd, "w");
	zassert_not_null(fp, "fdopen() failed, errno=%d", errno);
	zassert_equal(fileno(fp), efd, "fileno(fdopen(%d)) != %d", efd, efd);

	n = (ssize_t)fwrite(&write_val, sizeof(write_val), 1, fp);
	zassert_equal(n, 1, "fwrite() wrote %zd (errno=%d)", n, errno);
	zassert_ok(fflush(fp));

	n = read(efd, &read_val, sizeof(read_val));
	zassert_equal(n, (ssize_t)sizeof(read_val), "read() returned %zd (errno=%d)", n, errno);
	zassert_equal(read_val, write_val);

	zassert_ok(fclose(fp));
}

ZTEST(posix_device_io, test_fopen_fread_fwrite)
{
	static const char payload[] = "fopen-fread-fwrite";
	char buf[sizeof(payload)] = {0};
	FILE *fp;
	size_t len = strlen(payload);

	cleanup_test_file();

	fp = fopen(TEST_FILE, "w");
	zassert_not_null(fp, "fopen(w) failed, errno=%d", errno);
	zassert_equal(fwrite(payload, 1, len, fp), len);
	zassert_ok(fclose(fp));

	fp = fopen(TEST_FILE, "r");
	zassert_not_null(fp, "fopen(r) failed, errno=%d", errno);
	zassert_equal(fread(buf, 1, len, fp), len);
	zassert_mem_equal(buf, payload, len, "fread() data mismatch");
	zassert_ok(fclose(fp));

	cleanup_test_file();
}

ZTEST(posix_device_io, test_pread_pwrite)
{
	static const char payload[] = "pread-pwrite";
	char buf[sizeof(payload)] = {0};
	int fd;
	off_t pos;

	cleanup_test_file();

	fd = open(TEST_FILE, O_CREAT | O_RDWR, 0660);
	zassert_true(fd >= 0, "open() failed, errno=%d", errno);

	zassert_equal(pwrite(fd, payload, strlen(payload), 0), (ssize_t)strlen(payload));
	zassert_equal(pread(fd, buf, strlen(payload), 0), (ssize_t)strlen(payload));
	zassert_mem_equal(buf, payload, strlen(payload), "pread()/pwrite() data mismatch");

	pos = lseek(fd, 0, SEEK_CUR);
	zassert_equal(pos, 0, "offset I/O must not advance the file offset");

	zassert_ok(close(fd));
	cleanup_test_file();
}

ZTEST(posix_device_io, test_pread_pwrite_shm)
{
	static const char payload[] = "shm-io";
	char buf[sizeof(payload)] = {0};
	int fd;

	fd = shm_open("/device_io", O_RDWR | O_CREAT, 0666);
	zassert_true(fd >= 0, "shm_open() failed, errno=%d", errno);
	zassert_ok(ftruncate(fd, sizeof(payload)));

	zassert_equal(pwrite(fd, payload, strlen(payload), 0), (ssize_t)strlen(payload));
	zassert_equal(pread(fd, buf, strlen(payload), 0), (ssize_t)strlen(payload));
	zassert_mem_equal(buf, payload, strlen(payload), "pread()/pwrite() data mismatch");
	zassert_equal(lseek(fd, 0, SEEK_CUR), 0, "offset I/O must not advance the file offset");

	zassert_ok(close(fd));
	zassert_ok(shm_unlink("/device_io"));
}

ZTEST(posix_device_io, test_pread_pwrite_einval)
{
	char byte;
	int fd;

	fd = open(TEST_FILE, O_CREAT | O_RDWR, 0660);
	zassert_true(fd >= 0, "open() failed, errno=%d", errno);

	errno = 0;
	zassert_equal(pread(fd, &byte, 1, -1), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(pwrite(fd, &byte, 1, -1), -1);
	zassert_equal(errno, EINVAL);

	zassert_ok(close(fd));
	cleanup_test_file();
}

ZTEST(posix_device_io, test_poll)
{
	struct pollfd pfd;
	int efd;
	int ret;

	efd = eventfd(0, EFD_NONBLOCK);
	zassert_true(efd >= 0, "eventfd() failed, errno=%d", errno);

	pfd.fd = efd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	ret = poll(&pfd, 1, 0);
	zassert_true(ret >= 0, "poll ret %d errno=%d", ret, errno);
	zassert_equal(ret, 0);
	zassert_equal(pfd.revents, 0);

	zassert_ok(eventfd_write(efd, 1));

	pfd.revents = 0;
	ret = poll(&pfd, 1, 0);
	zassert_equal(ret, 1);
	zassert_true((pfd.revents & POLLIN) != 0);

	zassert_ok(close(efd));
}

ZTEST(posix_device_io, test_select_fd_macros)
{
	fd_set readfds;
	struct timeval tv = {0, 0};
	eventfd_t val;
	int efd;

	efd = eventfd(0, EFD_NONBLOCK);
	zassert_true(efd >= 0, "eventfd() failed, errno=%d", errno);

	FD_ZERO(&readfds);
	FD_SET(efd, &readfds);
	zassert_equal(select(efd + 1, &readfds, NULL, NULL, &tv), 0);
	zassert_false(FD_ISSET(efd, &readfds));

	zassert_ok(eventfd_write(efd, 1));

	FD_ZERO(&readfds);
	FD_SET(efd, &readfds);
	zassert_equal(select(efd + 1, &readfds, NULL, NULL, &tv), 1);
	zassert_true(FD_ISSET(efd, &readfds));

	FD_CLR(efd, &readfds);
	zassert_false(FD_ISSET(efd, &readfds));

	FD_ZERO(&readfds);
	FD_SET(efd, &readfds);
	zassert_equal(select(efd + 1, &readfds, NULL, NULL, NULL), 1);
	zassert_true(FD_ISSET(efd, &readfds));
	zassert_ok(eventfd_read(efd, &val));

	zassert_ok(close(efd));
}

ZTEST(posix_device_io, test_pselect)
{
	fd_set readfds;
	struct timespec ts = {0, 0};
	sigset_t mask;
	int efd;

	efd = eventfd(0, EFD_NONBLOCK);
	zassert_true(efd >= 0, "eventfd() failed, errno=%d", errno);

	sigemptyset(&mask);

	FD_ZERO(&readfds);
	FD_SET(efd, &readfds);
	zassert_equal(pselect(efd + 1, &readfds, NULL, NULL, &ts, &mask), 0);

	zassert_ok(eventfd_write(efd, 1));

	FD_ZERO(&readfds);
	FD_SET(efd, &readfds);
	zassert_equal(pselect(efd + 1, &readfds, NULL, NULL, &ts, NULL), 1);
	zassert_true(FD_ISSET(efd, &readfds));

	zassert_ok(close(efd));
}

ZTEST(posix_device_io, test_perror)
{
	errno = ENOENT;
	perror(NULL);

	errno = EIO;
	perror("posix_device_io");
}

ZTEST_SUITE(posix_device_io, NULL, test_mount, NULL, NULL, test_unmount);
