/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "symbolic_links_fd_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_symbolic_links_fd, test_symlinkat)
{
	int dirfd = open(TEST_ROOT, O_RDONLY | O_DIRECTORY);
	struct stat st;
	char buf[sizeof(TEST_CONTENT)];
	int fd;

	zassert_true(dirfd >= 0, "open(" TEST_ROOT ") failed: %d", errno);

	zassert_ok(symlinkat("file.txt", dirfd, "link"));
	zassert_ok(fstatat(dirfd, "link", &st, AT_SYMLINK_NOFOLLOW));
	zassert_true(S_ISLNK(st.st_mode));
	zassert_ok(fstatat(dirfd, "link", &st, 0));
	zassert_true(S_ISREG(st.st_mode));

	/* following the link reaches the file */
	fd = openat(dirfd, "link", O_RDONLY);
	zassert_true(fd >= 0, "openat(link) failed: %d", errno);
	zassert_equal(read(fd, buf, sizeof(buf) - 1), strlen(TEST_CONTENT));
	zassert_ok(close(fd));

	errno = 0;
	zassert_equal(symlinkat("file.txt", dirfd, "link"), -1);
	zassert_equal(errno, EEXIST);
	zassert_ok(unlinkat(dirfd, "link", 0));

	zassert_ok(symlinkat(TEST_FILE, AT_FDCWD, TEST_LINK));
	zassert_ok(unlink(TEST_LINK));

	errno = 0;
	zassert_equal(symlinkat("file.txt", fs_test_fd, "link"), -1);
	zassert_equal(errno, ENOTDIR);

	zassert_ok(close(dirfd));
}
