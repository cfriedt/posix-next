/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_fd, test_unlinkat)
{
	int dirfd = open_test_root();
	struct stat st;

	zassert_ok(unlinkat(dirfd, "empty.txt", 0));
	errno = 0;
	zassert_equal(fstatat(dirfd, "empty.txt", &st, 0), -1);
	zassert_equal(errno, ENOENT);

	/* a directory needs AT_REMOVEDIR, and must be empty */
	errno = 0;
	zassert_equal(unlinkat(dirfd, "dir", 0), -1);
	zassert_true(errno == EISDIR || errno == EPERM, "errno %d", errno);
	zassert_ok(unlinkat(dirfd, "dir/sub.txt", 0));
	zassert_ok(unlinkat(dirfd, "dir", AT_REMOVEDIR));
	errno = 0;
	zassert_equal(fstatat(dirfd, "dir", &st, 0), -1);
	zassert_equal(errno, ENOENT);

	errno = 0;
	zassert_equal(unlinkat(dirfd, "file.txt", AT_REMOVEDIR), -1);
	zassert_equal(errno, ENOTDIR);

	errno = 0;
	zassert_equal(unlinkat(dirfd, "nope.txt", 0), -1);
	zassert_equal(errno, ENOENT);

	zassert_ok(close(dirfd));
}
