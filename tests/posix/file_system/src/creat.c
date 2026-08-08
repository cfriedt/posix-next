/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_creat)
{
	struct stat st;
	int fd = creat(TEST_ROOT "/c.txt", 0600);

	zassert_true(fd >= 0);
	zassert_ok(close(fd));
	zassert_ok(stat(TEST_ROOT "/c.txt", &st));
	zassert_equal(st.st_size, 0);
	zassert_ok(unlink(TEST_ROOT "/c.txt"));
}
