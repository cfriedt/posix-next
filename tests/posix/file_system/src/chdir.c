/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_chdir)
{
	zassert_ok(chdir(TEST_DIR));

	/* relative resolution against the new working directory (Zephyr CWD) */
	IF_NOT_NATIVE_LIBC({
		struct stat st;

		zassert_ok(stat("sub.txt", &st));
	});

	errno = 0;
	zassert_equal(chdir(TEST_FILE), -1);
	zassert_equal(errno, ENOTDIR);

	errno = 0;
	zassert_equal(chdir(TEST_NOENT), -1);
	zassert_equal(errno, ENOENT);

	zassert_ok(chdir("/"));
}
