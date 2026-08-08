/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_mkdir)
{
	struct stat st;

	zassert_ok(mkdir(TEST_ROOT "/nd", 0777));
	zassert_ok(stat(TEST_ROOT "/nd", &st));
	zassert_true(S_ISDIR(st.st_mode));

	errno = 0;
	zassert_equal(mkdir(TEST_ROOT "/nd", 0777), -1);
	zassert_equal(errno, EEXIST);

	errno = 0;
	zassert_equal(mkdir(TEST_ROOT "/nothere/x", 0777), -1);
	zassert_equal(errno, ENOENT);

	zassert_ok(rmdir(TEST_ROOT "/nd"));
}
