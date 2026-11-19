/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_fd, test_fstatat)
{
	int dirfd = open_test_root();
	struct stat st;

	zassert_ok(fstatat(dirfd, "file.txt", &st, 0));
	zassert_true(S_ISREG(st.st_mode));
	zassert_equal(st.st_size, strlen(TEST_CONTENT));

	zassert_ok(fstatat(dirfd, "dir", &st, 0));
	zassert_true(S_ISDIR(st.st_mode));

	/* a regular file reports the same either way */
	zassert_ok(fstatat(dirfd, "file.txt", &st, AT_SYMLINK_NOFOLLOW));
	zassert_true(S_ISREG(st.st_mode));

	zassert_ok(fstatat(AT_FDCWD, TEST_FILE, &st, 0));
	zassert_true(S_ISREG(st.st_mode));

	errno = 0;
	zassert_equal(fstatat(dirfd, "nope.txt", &st, 0), -1);
	zassert_equal(errno, ENOENT);

	errno = 0;
	zassert_equal(fstatat(fs_test_fd, "file.txt", &st, 0), -1);
	zassert_equal(errno, ENOTDIR);

	zassert_ok(close(dirfd));
}
