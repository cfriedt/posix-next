/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

ZTEST_USER(posix_file_system, test_stat)
{
	struct stat st;

	zassert_ok(stat(TEST_FILE, &st));
	zassert_equal(st.st_size, strlen(TEST_CONTENT));
	zassert_true(S_ISREG(st.st_mode));
	zassert_equal(st.st_nlink, 1);

	zassert_ok(stat(TEST_DIR, &st));
	zassert_true(S_ISDIR(st.st_mode));

	zassert_ok(stat(TEST_EMPTY, &st));
	zassert_equal(st.st_size, 0);

	/* identity: the same file is the same file, other files differ */
	{
		struct stat other;

		zassert_ok(stat(TEST_FILE, &other));
		zassert_equal(st.st_dev, other.st_dev);
		zassert_not_equal(st.st_ino, other.st_ino);
		zassert_ok(stat(TEST_EMPTY, &other));
		zassert_equal(st.st_ino, other.st_ino);
	}

	errno = 0;
	zassert_equal(stat(TEST_NOENT, &st), -1);
	zassert_equal(errno, ENOENT);
}
