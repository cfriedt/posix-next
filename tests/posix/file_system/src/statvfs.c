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

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the root exists: a read-only directory of mount points with no blocks of its own */
		zassert_ok(statvfs("/", &sv));
		zassert_true(sv.f_bsize > 0);
		zassert_equal(sv.f_blocks, 0);
		zassert_true((sv.f_flag & ST_RDONLY) != 0);
		zassert_true(sv.f_namemax > 0);
	}
}
