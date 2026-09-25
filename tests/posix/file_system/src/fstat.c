/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <string.h>
#include <sys/stat.h>

ZTEST_USER(posix_file_system, test_fstat)
{
	struct stat st;

	zassert_ok(fstat(fs_test_fd, &st));
	zassert_equal(st.st_size, strlen(TEST_CONTENT));
	zassert_true(S_ISREG(st.st_mode));

	/* identity: the descriptor names the same file as its path */
	{
		struct stat by_path;

		zassert_ok(stat(TEST_FILE, &by_path));
		zassert_equal(st.st_dev, by_path.st_dev);
		zassert_equal(st.st_ino, by_path.st_ino);
	}
	zassert_equal(fstat(fs_test_rofd, &st), 0);
}
