/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_access)
{
	zassert_ok(access(TEST_FILE, F_OK));
	zassert_ok(access(TEST_FILE, R_OK | W_OK));
	zassert_ok(access(TEST_DIR, X_OK));

	errno = 0;
	zassert_equal(access(TEST_NOENT, F_OK), -1);
	zassert_equal(errno, ENOENT);
}
