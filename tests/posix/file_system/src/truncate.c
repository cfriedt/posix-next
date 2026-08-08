/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_truncate)
{
	struct stat st;

	zassert_ok(truncate(TEST_FILE, 4));
	zassert_ok(stat(TEST_FILE, &st));
	zassert_equal(st.st_size, 4);

	errno = 0;
	zassert_equal(truncate(TEST_FILE, -1), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(truncate(TEST_NOENT, 0), -1);
	zassert_equal(errno, ENOENT);
}
