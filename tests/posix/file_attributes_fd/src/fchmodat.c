/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_fd_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static mode_t perm_of(const char *path)
{
	struct stat st;

	zassert_ok(stat(path, &st), "stat %s: %d", path, errno);

	return st.st_mode & 07777;
}

ZTEST_USER(posix_file_attributes_fd, test_fchmodat)
{
	int dirfd = open(TEST_ROOT, O_RDONLY | O_DIRECTORY);

	zassert_true(dirfd >= 0, "open(" TEST_ROOT ") failed: %d", errno);

	zassert_ok(fchmodat(dirfd, "file.txt", 0644, 0));
	zassert_equal(perm_of(TEST_FILE), 0644, "0%o", perm_of(TEST_FILE));
	/* the flag only matters for a symbolic link */
	zassert_ok(fchmodat(dirfd, "dir", 0755, AT_SYMLINK_NOFOLLOW));
	zassert_equal(perm_of(TEST_DIR), 0755, "0%o", perm_of(TEST_DIR));
	zassert_ok(fchmodat(AT_FDCWD, TEST_FILE, 0600, 0));
	zassert_equal(perm_of(TEST_FILE), 0600, "0%o", perm_of(TEST_FILE));

	errno = 0;
	zassert_equal(fchmodat(dirfd, "nope.txt", 0644, 0), -1);
	zassert_equal(errno, ENOENT);

	errno = 0;
	zassert_equal(fchmodat(fs_test_fd, "file.txt", 0644, 0), -1);
	zassert_equal(errno, ENOTDIR);

	errno = 0;
	zassert_equal(fchmodat(dirfd, "file.txt", 0644, AT_REMOVEDIR), -1);
	zassert_equal(errno, EINVAL);

	zassert_ok(close(dirfd));
}
