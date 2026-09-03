/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "symbolic_links_fd_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

ZTEST_USER(posix_symbolic_links_fd, test_readlinkat)
{
	int dirfd = open(TEST_ROOT, O_RDONLY | O_DIRECTORY);
	char buf[32];
	ssize_t len;

	zassert_true(dirfd >= 0, "open(" TEST_ROOT ") failed: %d", errno);
	zassert_ok(symlinkat("file.txt", dirfd, "link"));

	len = readlinkat(dirfd, "link", buf, sizeof(buf));
	zassert_equal(len, strlen("file.txt"), "readlinkat: %zd (errno %d)", len, errno);
	zassert_mem_equal(buf, "file.txt", len);

	len = readlinkat(AT_FDCWD, TEST_LINK, buf, sizeof(buf));
	zassert_equal(len, strlen("file.txt"), "readlinkat: %zd (errno %d)", len, errno);

	/* a short buffer truncates */
	len = readlinkat(dirfd, "link", buf, 4);
	zassert_equal(len, 4, "readlinkat: %zd (errno %d)", len, errno);
	zassert_mem_equal(buf, "file", 4);

	errno = 0;
	zassert_equal(readlinkat(dirfd, "file.txt", buf, sizeof(buf)), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(readlinkat(dirfd, "nope", buf, sizeof(buf)), -1);
	zassert_equal(errno, ENOENT);

	zassert_ok(unlinkat(dirfd, "link", 0));
	zassert_ok(close(dirfd));
}
