/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_unlink)
{
	zassert_ok(unlink(TEST_FILE));

	errno = 0;
	zassert_equal(unlink(TEST_FILE), -1);
	zassert_equal(errno, ENOENT);

	errno = 0;
	zassert_equal(unlink(TEST_DIR), -1);
	zassert_true(errno == EISDIR || errno == EPERM, "errno %d", errno);
}
