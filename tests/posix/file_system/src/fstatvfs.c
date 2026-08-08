/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <sys/statvfs.h>

ZTEST_USER(posix_file_system, test_fstatvfs)
{
	struct statvfs a, b;

	zassert_ok(fstatvfs(fs_test_fd, &a));
	zassert_ok(statvfs(TEST_FILE, &b));
	zassert_equal(a.f_bsize, b.f_bsize);
}
