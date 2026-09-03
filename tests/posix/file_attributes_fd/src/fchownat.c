/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_fd_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

ZTEST_USER(posix_file_attributes_fd, test_fchownat)
{
	int dirfd = open(TEST_ROOT, O_RDONLY | O_DIRECTORY);

	zassert_true(dirfd >= 0, "open(" TEST_ROOT ") failed: %d", errno);

	/* the owning identity (a host user cannot give files away), and "leave unchanged" */
	zassert_ok(fchownat(dirfd, "file.txt", IS_ENABLED(CONFIG_NATIVE_LIBC) ? (uid_t)-1 : 0,
			    IS_ENABLED(CONFIG_NATIVE_LIBC) ? (gid_t)-1 : 0, 0));
	zassert_ok(fchownat(dirfd, "file.txt", (uid_t)-1, (gid_t)-1, AT_SYMLINK_NOFOLLOW));
	zassert_ok(fchownat(AT_FDCWD, TEST_FILE, (uid_t)-1, (gid_t)-1, 0));

	errno = 0;
	zassert_equal(fchownat(dirfd, "nope.txt", (uid_t)-1, (gid_t)-1, 0), -1);
	zassert_equal(errno, ENOENT);

	errno = 0;
	zassert_equal(fchownat(fs_test_fd, "file.txt", (uid_t)-1, (gid_t)-1, 0), -1);
	zassert_equal(errno, ENOTDIR);

	zassert_ok(close(dirfd));
}
