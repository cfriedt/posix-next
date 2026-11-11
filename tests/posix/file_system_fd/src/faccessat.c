/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_fd, test_faccessat)
{
	int dirfd = open_test_root();

	zassert_ok(faccessat(dirfd, "file.txt", F_OK, 0));
	zassert_ok(faccessat(dirfd, "file.txt", R_OK | W_OK, 0));
	zassert_ok(faccessat(dirfd, "dir", F_OK, AT_EACCESS));
	zassert_ok(faccessat(AT_FDCWD, TEST_FILE, F_OK, 0));

	errno = 0;
	zassert_equal(faccessat(dirfd, "nope.txt", F_OK, 0), -1);
	zassert_equal(errno, ENOENT);

	errno = 0;
	zassert_equal(faccessat(fs_test_fd, "file.txt", F_OK, 0), -1);
	zassert_equal(errno, ENOTDIR);

	zassert_ok(close(dirfd));
}
