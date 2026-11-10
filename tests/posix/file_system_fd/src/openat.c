/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_fd, test_openat)
{
	int dirfd = open_test_root();
	int fd;
	char buf[sizeof(TEST_CONTENT)];

	/* relative to the directory descriptor */
	fd = openat(dirfd, "file.txt", O_RDONLY);
	zassert_true(fd >= 0, "openat failed: %d", errno);
	zassert_equal(read(fd, buf, sizeof(buf) - 1), strlen(TEST_CONTENT));
	buf[strlen(TEST_CONTENT)] = '\0';
	zassert_str_equal(buf, TEST_CONTENT);
	zassert_ok(close(fd));

	/* an absolute path ignores the descriptor */
	fd = openat(fs_test_fd, TEST_FILE, O_RDONLY);
	zassert_true(fd >= 0, "openat(absolute) failed: %d", errno);
	zassert_ok(close(fd));

	/* creation, then the same name is found through the descriptor */
	fd = openat(dirfd, "new.txt", O_WRONLY | O_CREAT | O_EXCL, 0600);
	zassert_true(fd >= 0, "openat(O_CREAT) failed: %d", errno);
	zassert_ok(close(fd));
	fd = openat(dirfd, "new.txt", O_RDONLY);
	zassert_true(fd >= 0);
	zassert_ok(close(fd));
	zassert_ok(unlink(TEST_ROOT "/new.txt"));

	errno = 0;
	zassert_equal(openat(dirfd, "nope.txt", O_RDONLY), -1);
	zassert_equal(errno, ENOENT);

	errno = 0;
	zassert_equal(openat(fs_test_fd, "file.txt", O_RDONLY), -1);
	zassert_equal(errno, ENOTDIR);

	errno = 0;
	zassert_equal(openat(-1, "file.txt", O_RDONLY), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(close(dirfd));
}
