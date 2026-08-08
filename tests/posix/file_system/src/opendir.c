/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <dirent.h>
#include <errno.h>

ZTEST_USER(posix_file_system, test_opendir)
{
	DIR *d;

	d = opendir(TEST_DIR);
	zassert_not_null(d);
	zassert_ok(closedir(d));

	errno = 0;
	zassert_is_null(opendir(TEST_NOENT));
	zassert_true(errno == ENOENT || errno == ENOTDIR, "errno %d", errno);
}
