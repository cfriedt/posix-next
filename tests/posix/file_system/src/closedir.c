/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <dirent.h>
#include <errno.h>

ZTEST_USER(posix_file_system, test_closedir)
{
	DIR *d = opendir(TEST_DIR);

	zassert_not_null(d);
	zassert_ok(closedir(d));

	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(closedir(NULL), -1);
		zassert_equal(errno, EBADF);
	});
}
