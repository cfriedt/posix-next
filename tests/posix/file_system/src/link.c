/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_link)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the host filesystem supports hard links */
		zassert_ok(link(TEST_FILE, TEST_NOENT));
		zassert_ok(unlink(TEST_NOENT));
	} else {
		errno = 0;
		zassert_equal(link(TEST_FILE, TEST_NOENT), -1);
		zassert_equal(errno, EPERM);
	}
}
