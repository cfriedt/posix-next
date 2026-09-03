/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_fd, test_mkdirat)
{
	int dirfd = open_test_root();
	struct stat st;

	zassert_ok(mkdirat(dirfd, "nd", 0777));
	zassert_ok(stat(TEST_ROOT "/nd", &st));
	zassert_true(S_ISDIR(st.st_mode));

	errno = 0;
	zassert_equal(mkdirat(dirfd, "nd", 0777), -1);
	zassert_equal(errno, EEXIST);

	errno = 0;
	zassert_equal(mkdirat(dirfd, "nothere/x", 0777), -1);
	zassert_equal(errno, ENOENT);

	errno = 0;
	zassert_equal(mkdirat(fs_test_fd, "nd2", 0777), -1);
	zassert_equal(errno, ENOTDIR);

	zassert_ok(rmdir(TEST_ROOT "/nd"));
	zassert_ok(close(dirfd));
}
