/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_fd, test_renameat)
{
	int dirfd = open_test_root();
	int subfd;
	struct stat st;

	zassert_ok(renameat(dirfd, "empty.txt", dirfd, "moved.txt"));
	zassert_ok(fstatat(dirfd, "moved.txt", &st, 0));
	errno = 0;
	zassert_equal(fstatat(dirfd, "empty.txt", &st, 0), -1);
	zassert_equal(errno, ENOENT);

	/* across directory descriptors */
	subfd = open(TEST_DIR, O_RDONLY | O_DIRECTORY);
	zassert_true(subfd >= 0);
	zassert_ok(renameat(dirfd, "moved.txt", subfd, "moved.txt"));
	zassert_ok(fstatat(subfd, "moved.txt", &st, 0));
	zassert_ok(unlinkat(subfd, "moved.txt", 0));
	zassert_ok(close(subfd));

	errno = 0;
	zassert_equal(renameat(dirfd, "nope.txt", dirfd, "other.txt"), -1);
	zassert_equal(errno, ENOENT);

	zassert_ok(close(dirfd));
}
