/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_getcwd)
{
	char buf[64];

	zassert_ok(chdir("/"));
	zassert_equal_ptr(getcwd(buf, sizeof(buf)), buf);
	zassert_str_equal(buf, "/");

	IF_NOT_NATIVE_LIBC({
		zassert_ok(chdir(TEST_DIR));
		zassert_not_null(getcwd(buf, sizeof(buf)));
		zassert_str_equal(buf, TEST_DIR);

		errno = 0;
		zassert_is_null(getcwd(buf, 2));
		zassert_equal(errno, ERANGE);
	});

	zassert_ok(chdir("/"));
}
