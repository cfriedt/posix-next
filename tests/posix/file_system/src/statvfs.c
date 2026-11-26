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
		/* the suite's volume is mounted at the root: the root is that file system */
		struct statvfs root;

		zassert_ok(statvfs("/", &root));
		zassert_equal(root.f_bsize, sv.f_bsize);
		zassert_equal(root.f_blocks, sv.f_blocks);
		zassert_true((root.f_flag & ST_RDONLY) == 0);
		zassert_true(root.f_namemax > 0);
	}
}
