/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_fd, test_linkat)
{
	int dirfd = open_test_root();

	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the host filesystem supports hard links */
		zassert_ok(linkat(dirfd, "file.txt", dirfd, "nope.txt", 0));
		zassert_ok(unlinkat(dirfd, "nope.txt", 0));
	} else {
		errno = 0;
		zassert_equal(linkat(dirfd, "file.txt", dirfd, "nope.txt", 0), -1);
		zassert_equal(errno, EPERM);
	}

	zassert_ok(close(dirfd));
}
