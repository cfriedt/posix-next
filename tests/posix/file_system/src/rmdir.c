/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_rmdir)
{
	errno = 0;
	zassert_equal(rmdir(TEST_FILE), -1);
	zassert_equal(errno, ENOTDIR, "expected ENOTDIR got %d", errno);

	errno = 0;
	zassert_equal(rmdir(TEST_DIR), -1);
	zassert_true(errno == ENOTEMPTY || errno == EACCES, "errno %d", errno);

	zassert_ok(unlink(TEST_SUB));
	zassert_ok(rmdir(TEST_DIR));

	errno = 0;
	zassert_equal(rmdir(TEST_NOENT), -1);
	zassert_equal(errno, ENOENT);
}
