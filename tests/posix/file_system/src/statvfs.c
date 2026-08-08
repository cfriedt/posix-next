/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <sys/statvfs.h>

ZTEST_USER(posix_file_system, test_statvfs)
{
	struct statvfs sv;

	zassert_ok(statvfs(TEST_FILE, &sv));
	zassert_true(sv.f_bsize > 0);
	zassert_true(sv.f_blocks > 0);
}
