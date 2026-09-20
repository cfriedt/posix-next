/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fifo_fd_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_fifo_fd, test_mknodat)
{
	int dirfd = open(TEST_ROOT, O_RDONLY | O_DIRECTORY);
	struct stat st;

	zassert_true(dirfd >= 0, "open(" TEST_ROOT ") failed: %d", errno);

	zassert_ok(mknodat(dirfd, "fifo", S_IFIFO | 0644, 0));
	zassert_ok(fstatat(dirfd, "fifo", &st, 0));
	zassert_true(S_ISFIFO(st.st_mode));
	errno = 0;
	zassert_equal(mknodat(dirfd, "fifo", S_IFIFO | 0644, 0), -1);
	zassert_equal(errno, EEXIST);
	zassert_ok(unlinkat(dirfd, "fifo", 0));

	/* a regular file, empty */
	zassert_ok(mknodat(dirfd, "reg", S_IFREG | 0644, 0));
	zassert_ok(fstatat(dirfd, "reg", &st, 0));
	zassert_true(S_ISREG(st.st_mode));
	zassert_equal(st.st_size, 0);
	errno = 0;
	zassert_equal(mknodat(dirfd, "reg", S_IFREG | 0644, 0), -1);
	zassert_equal(errno, EEXIST);
	zassert_ok(unlinkat(dirfd, "reg", 0));

	/* no device special files exist to make (the host has them) */
	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(mknodat(dirfd, "chr", S_IFCHR | 0644, 0), -1);
		zassert_equal(errno, EINVAL);
	});

	zassert_ok(close(dirfd));
}
