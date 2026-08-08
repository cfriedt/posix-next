/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <stdio.h>

ZTEST_USER(posix_file_system, test_remove)
{
	zassert_ok(remove(TEST_FILE));

	zassert_ok(remove(TEST_SUB));
	zassert_ok(remove(TEST_DIR));

	errno = 0;
	zassert_equal(remove(TEST_NOENT), -1);
	zassert_equal(errno, ENOENT);
}
