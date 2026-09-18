/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_tests.h"

#include <errno.h>
#include <sys/stat.h>

ZTEST_USER(posix_file_attributes, test_fchmod)
{
	zassert_ok(fchmod(fs_test_fd, 0644));
	zassert_ok(fchmod(fs_test_fd, 0600));
	/* a mode change needs ownership, not write access */
	zassert_ok(fchmod(fs_test_rofd, 0600));

	errno = 0;
	zassert_equal(fchmod(-1, 0644), -1);
	zassert_equal(errno, EBADF);
}
