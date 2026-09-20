/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fifo_fd_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_fifo_fd, test_mkfifoat)
{
	int dirfd = open(TEST_ROOT, O_RDONLY | O_DIRECTORY);
	int fd = open(TEST_FILE, O_RDONLY);
	struct stat st;

	zassert_true(dirfd >= 0, "open(" TEST_ROOT ") failed: %d", errno);
	zassert_true(fd >= 0, "open(" TEST_FILE ") failed: %d", errno);

	zassert_ok(mkfifoat(dirfd, "fifo", 0644));
	zassert_ok(fstatat(dirfd, "fifo", &st, 0));
	zassert_true(S_ISFIFO(st.st_mode));
	errno = 0;
	zassert_equal(mkfifoat(AT_FDCWD, TEST_FIFO, 0644), -1);
	zassert_equal(errno, EEXIST);
	zassert_ok(unlinkat(dirfd, "fifo", 0));

	zassert_ok(mkfifoat(AT_FDCWD, TEST_FIFO, 0644));
	zassert_ok(stat(TEST_FIFO, &st));
	zassert_true(S_ISFIFO(st.st_mode));
	zassert_ok(unlink(TEST_FIFO));

	errno = 0;
	zassert_equal(mkfifoat(fd, "fifo", 0644), -1);
	zassert_equal(errno, ENOTDIR);

	zassert_ok(close(fd));
	zassert_ok(close(dirfd));
}
