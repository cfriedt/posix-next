/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_tests.h"

#include <errno.h>
#include <sys/stat.h>

ZTEST_USER(posix_file_attributes, test_chmod)
{
	zassert_ok(chmod(TEST_FILE, 0644));
	zassert_ok(chmod(TEST_FILE, 0600));
	zassert_ok(chmod(TEST_DIR, 0755));

	errno = 0;
	zassert_equal(chmod(TEST_NOENT, 0644), -1);
	zassert_equal(errno, ENOENT);
}
