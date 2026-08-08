/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

ZTEST_USER(posix_file_system, test_rename)
{
	struct stat st;

	zassert_ok(rename(TEST_FILE, TEST_NOENT));
	zassert_ok(stat(TEST_NOENT, &st));
	errno = 0;
	zassert_equal(stat(TEST_FILE, &st), -1);
	zassert_equal(errno, ENOENT);
	zassert_ok(rename(TEST_NOENT, TEST_FILE));
}
