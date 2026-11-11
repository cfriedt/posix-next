/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_tests.h"

#include <errno.h>
#include <sys/stat.h>

ZTEST_USER(posix_file_attributes, test_fchmod)
{
	struct stat st;

	zassert_ok(fchmod(fs_test_fd, 0644));
	zassert_ok(fstat(fs_test_fd, &st));
	zassert_equal(st.st_mode & 07777, 0644, "0%o", st.st_mode & 07777);
	zassert_ok(stat(TEST_FILE, &st));
	zassert_equal(st.st_mode & 07777, 0644, "0%o", st.st_mode & 07777);

	/* a mode change needs ownership, not write access */
	zassert_ok(fchmod(fs_test_rofd, 0600));
	zassert_ok(fstat(fs_test_rofd, &st));
	zassert_equal(st.st_mode & 07777, 0600, "0%o", st.st_mode & 07777);

	errno = 0;
	zassert_equal(fchmod(-1, 0644), -1);
	zassert_equal(errno, EBADF);
}
