/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2024 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"
#include "test_fs.h"

#define EFD_INTERNAL_FLAG 0x1 /* ZVFS_EFD_IN_USE; not settable via F_SETFL */

static void cleanup_test_file(void)
{
	(void)unlink(TEST_FILE);
}

static int open_test_file(void)
{
	int fd;

	cleanup_test_file();

	fd = open(TEST_FILE, O_CREAT | O_RDWR | O_TRUNC, 0660);
	zassert_true(fd >= 0, "open() failed, errno=%d", errno);

	return fd;
}

ZTEST(posix_fd_mgmt, test_dup)
{
	static const char payload[] = "dup-file";
	const size_t len = strlen(payload);
	char buf_a[sizeof(payload)];
	char buf_b[sizeof(payload)];
	int fd0;
	int dupfd;

	errno = 0;
	zassert_equal(dup(-1), -1);
	zassert_equal(errno, EBADF);

	fd0 = open_test_file();
	dupfd = dup(fd0);
	zassert_true(dupfd >= 0, "dup() failed: %d", errno);
	zassert_not_equal(dupfd, fd0, "dup() returned the same fd");

	zassert_equal(write(fd0, payload, len), (ssize_t)len);
	zassert_equal(lseek(fd0, 0, SEEK_SET), 0);

	/* dup'd fds share one file offset (POSIX open file description). */
	{
		const size_t split = 3;

		zassert_equal(read(fd0, buf_a, split), (ssize_t)split);
		zassert_equal(read(dupfd, buf_b, len - split), (ssize_t)(len - split));
		buf_a[split] = '\0';
		buf_b[len - split] = '\0';
		zassert_mem_equal(buf_a, payload, split);
		zassert_mem_equal(buf_b, payload + split, len - split);
	}

	zassert_ok(close(dupfd));
	zassert_ok(close(fd0));
	cleanup_test_file();
}

ZTEST(posix_fd_mgmt, test_dup2)
{
	eventfd_t val = 19;
	eventfd_t read_val = 0;
	const int target_fd = 10;
	int efd;
	int fd0;

	fd0 = open_test_file();

	errno = 0;
	zassert_equal(dup2(-1, -1), -1);
	zassert_equal(errno, EBADF);

	errno = 0;
	zassert_equal(dup2(fd0, -1), -1);
	zassert_equal(errno, EBADF);

	errno = 0;
	zassert_equal(dup2(-1, fd0), -1);
	zassert_equal(errno, EBADF);

	zassert_equal(dup2(fd0, target_fd), target_fd, "dup2() returned %d", errno);

	zassert_ok(close(target_fd));
	zassert_ok(close(fd0));
	cleanup_test_file();

	efd = eventfd(0, 0);
	zassert_true(efd >= 0, "eventfd() failed, errno=%d", errno);

	zassert_equal(dup2(efd, target_fd), target_fd, "dup2() returned %d", errno);

	zassert_ok(eventfd_write(target_fd, val));
	zassert_ok(eventfd_read(efd, &read_val));
	zassert_equal(read_val, val, "read_val == %lld", read_val);

	zassert_ok(close(target_fd));
	zassert_ok(close(efd));
}

ZTEST(posix_fd_mgmt, test_fcntl)
{
	eventfd_t val = 42;
	eventfd_t read_val = 0;
	const int minfd = 10;
	int efd;
	int dupfd;
	int file_fd;
	int flags;

	errno = 0;
	zassert_equal(fcntl(-1, F_GETFL), -1);
	zassert_equal(errno, EBADF);

	errno = 0;
	zassert_equal(fcntl(-1, F_DUPFD, 0), -1);
	zassert_equal(errno, EBADF);

	efd = eventfd(0, 0);
	zassert_true(efd >= 0, "eventfd() failed, errno=%d", errno);

	dupfd = fcntl(efd, F_DUPFD, 0);
	zassert_true(dupfd >= 0, "fcntl(F_DUPFD) failed, errno=%d", errno);
	zassert_not_equal(dupfd, efd, "fcntl(F_DUPFD) returned the same fd");

	zassert_ok(eventfd_write(dupfd, val));
	zassert_ok(eventfd_read(efd, &read_val));
	zassert_equal(read_val, val, "read_val == %lld", read_val);
	zassert_ok(close(dupfd));

	dupfd = fcntl(efd, F_DUPFD, minfd);
	zassert_true(dupfd >= minfd, "fcntl(F_DUPFD, %d) returned %d", minfd, dupfd);
	zassert_ok(close(dupfd));

	IF_NOT_NATIVE_LIBC({
		flags = fcntl(efd, F_GETFL);
		zassert_equal(flags, 0, "fcntl(F_GETFL) == %d", flags);

		zassert_ok(fcntl(efd, F_SETFL, O_NONBLOCK),
			   "fcntl(F_SETFL, O_NONBLOCK) failed, errno=%d", errno);

		flags = fcntl(efd, F_GETFL);
		zassert_equal(flags, O_NONBLOCK, "fcntl(F_GETFL) == %d", flags);

		zassert_ok(fcntl(efd, F_SETFL, 0), "fcntl(F_SETFL, 0) failed, errno=%d", errno);

		flags = fcntl(efd, F_GETFL);
		zassert_equal(flags, 0, "fcntl(F_GETFL) == %d", flags);

		errno = 0;
		zassert_equal(fcntl(efd, F_SETFL, EFD_INTERNAL_FLAG), -1);
		zassert_equal(errno, EINVAL);

#ifdef F_GETFD
		flags = fcntl(efd, F_GETFD);
		zassert_equal(flags, 0, "fcntl(F_GETFD) == %d", flags);
#endif
	});

	file_fd = open_test_file();

#ifdef F_GETFD
	flags = fcntl(file_fd, F_GETFD);
	zassert_equal(flags, 0, "fcntl(F_GETFD) == %d", flags);
	zassert_ok(fcntl(file_fd, F_SETFD, FD_CLOEXEC), "fcntl(F_SETFD) failed, errno=%d", errno);
	flags = fcntl(file_fd, F_GETFD);
	zassert_equal(flags, FD_CLOEXEC, "fcntl(F_GETFD) == %d", flags);
#endif

#if defined(CONFIG_NATIVE_LIBC)
	flags = fcntl(file_fd, F_GETFL);
	zassert_true(flags >= 0, "fcntl(F_GETFL) on file failed, errno=%d", errno);
#else
	errno = 0;
	zassert_equal(fcntl(file_fd, F_GETFL), -1);
	zassert_equal(errno, EOPNOTSUPP);

#ifdef F_SETLK
	{
		struct flock fl = {
			.l_type = F_WRLCK,
			.l_whence = SEEK_SET,
			.l_start = 0,
			.l_len = 10,
		};

		zassert_ok(fcntl(file_fd, F_SETLK, &fl), "fcntl(F_SETLK) failed, errno=%d", errno);

		dupfd = dup(file_fd);
		zassert_true(dupfd >= 0, "dup() failed, errno=%d", errno);

		fl = (struct flock){
			.l_type = F_UNLCK,
			.l_whence = SEEK_SET,
			.l_start = 0,
			.l_len = 10,
		};
		zassert_ok(fcntl(dupfd, F_SETLK, &fl),
			   "fcntl(F_SETLK, F_UNLCK) on dup failed, errno=%d", errno);

		fl = (struct flock){
			.l_type = F_WRLCK,
			.l_whence = SEEK_SET,
			.l_start = 0,
			.l_len = 10,
		};
		zassert_ok(fcntl(file_fd, F_GETLK, &fl), "fcntl(F_GETLK) failed, errno=%d", errno);
		zassert_equal(fl.l_type, F_UNLCK);

		zassert_ok(close(dupfd));
	}
#endif /* F_SETLK */
#endif

	zassert_ok(close(file_fd));
	cleanup_test_file();
	zassert_ok(close(efd));
}

ZTEST(posix_fd_mgmt, test_dup_eventfd)
{
	eventfd_t val = 77;
	eventfd_t read_val = 0;
	int efd;
	int dupfd;

	efd = eventfd(0, 0);
	zassert_true(efd >= 0, "eventfd() failed, errno=%d", errno);

	dupfd = dup(efd);
	zassert_true(dupfd >= 0, "dup() failed, errno=%d", errno);
	zassert_not_equal(dupfd, efd, "dup() returned the same fd");

	zassert_ok(eventfd_write(efd, val));
	zassert_ok(eventfd_read(dupfd, &read_val));
	zassert_equal(read_val, val, "read_val == %lld", read_val);

	zassert_ok(close(dupfd));
	zassert_ok(close(efd));
}

ZTEST(posix_fd_mgmt, test_ftruncate)
{
	static const char payload[] = "ftruncate-test";
	const size_t len = strlen(payload);
	const off_t truncated = 4;
	int fd;

	zassert_equal(ftruncate(-1, 0), -1);

	fd = open_test_file();

	zassert_equal(write(fd, payload, len), (ssize_t)len);
	zassert_ok(ftruncate(fd, truncated));
	zassert_equal(lseek(fd, 0, SEEK_END), truncated);

	zassert_ok(close(fd));
	cleanup_test_file();
}

ZTEST(posix_fd_mgmt, test_lseek)
{
	static const char payload[] = "lseek-test";
	const size_t len = strlen(payload);
	int fd;

	zassert_equal(lseek(-1, 0, SEEK_SET), (off_t)-1);

	fd = open_test_file();

	zassert_equal(write(fd, payload, len), (ssize_t)len);
	zassert_equal(lseek(fd, 0, SEEK_SET), 0);
	zassert_equal(lseek(fd, (off_t)len, SEEK_SET), (off_t)len);
	zassert_equal(lseek(fd, 0, SEEK_END), (off_t)len);
	zassert_equal(lseek(fd, -2, SEEK_END), (off_t)(len - 2));

	zassert_ok(close(fd));
	cleanup_test_file();
}

ZTEST(posix_fd_mgmt, test_fseeko_ftello)
{
	static const char payload[] = "fseeko-ftello";
	char buf[sizeof(payload)] = {0};
	FILE *fp;
	off_t pos;
	size_t len = strlen(payload);

	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(fseeko(NULL, 0, SEEK_SET), -1);
		zassert_equal(errno, EBADF);
		errno = 0;
		zassert_equal(ftello(NULL), (off_t)-1);
		zassert_equal(errno, EBADF);
	});

	cleanup_test_file();

	fp = fopen(TEST_FILE, "w");
	zassert_not_null(fp, "fopen(w) failed, errno=%d", errno);
	zassert_equal(fwrite(payload, 1, len, fp), len);
	zassert_ok(fclose(fp));

	fp = fopen(TEST_FILE, "r+");
	zassert_not_null(fp, "fopen(r+) failed, errno=%d", errno);

	zassert_ok(fseeko(fp, 0, SEEK_SET));
	pos = ftello(fp);
	zassert_equal(pos, 0, "ftello(SEEK_SET) == %lld", (long long)pos);

	zassert_ok(fseeko(fp, 0, SEEK_END));
	pos = ftello(fp);
	zassert_equal(pos, (off_t)len, "ftello(SEEK_END) == %lld", (long long)pos);

	zassert_ok(fseeko(fp, 0, SEEK_SET));
	zassert_equal(fread(buf, 1, len, fp), len);
	zassert_mem_equal(buf, payload, len, "fread() data mismatch");

	zassert_ok(fclose(fp));
	cleanup_test_file();
}

ZTEST(posix_fd_mgmt, test_fseek_ftell_rewind)
{
	static const char payload[] = "fseek-ftell";
	char buf[sizeof(payload)] = {0};
	FILE *fp;
	long pos;
	fpos_t fpos;
	size_t len = strlen(payload);

	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(fseek(NULL, 0, SEEK_SET), -1);
		zassert_equal(errno, EBADF);
		errno = 0;
		zassert_equal(ftell(NULL), -1L);
		zassert_equal(errno, EBADF);
	});

	cleanup_test_file();

	fp = fopen(TEST_FILE, "w");
	zassert_not_null(fp, "fopen(w) failed, errno=%d", errno);
	zassert_equal(fwrite(payload, 1, len, fp), len);
	zassert_ok(fclose(fp));

	fp = fopen(TEST_FILE, "r+");
	zassert_not_null(fp, "fopen(r+) failed, errno=%d", errno);

	zassert_ok(fseek(fp, 0, SEEK_SET));
	pos = ftell(fp);
	zassert_equal(pos, 0, "ftell(SEEK_SET) == %ld", pos);

	zassert_ok(fseek(fp, 0, SEEK_END));
	pos = ftell(fp);
	zassert_equal(pos, (long)len, "ftell(SEEK_END) == %ld", pos);

	rewind(fp);
	pos = ftell(fp);
	zassert_equal(pos, 0, "ftell after rewind == %ld", pos);

	zassert_ok(fgetpos(fp, &fpos));
	zassert_equal(ftell(fp), 0);

	zassert_ok(fsetpos(fp, &fpos));
	zassert_equal(fread(buf, 1, len, fp), len);
	zassert_mem_equal(buf, payload, len, "fread() data mismatch");

	zassert_ok(fclose(fp));
	cleanup_test_file();
}

ZTEST_SUITE(posix_fd_mgmt, NULL, test_mount, NULL, NULL, test_unmount);
