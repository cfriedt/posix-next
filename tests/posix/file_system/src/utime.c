/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <utime.h>

ZTEST_USER(posix_file_system, test_utime)
{
	int rc = utime(TEST_FILE, NULL);

	if (rc < 0) {
		/* backends without timestamp support report ENOTSUP */
		zassert_equal(errno, ENOTSUP, "errno %d", errno);
	}

	errno = 0;
	zassert_equal(utime(TEST_NOENT, NULL), -1);
	zassert_true(errno == ENOENT || errno == ENOTSUP, "errno %d", errno);
}
