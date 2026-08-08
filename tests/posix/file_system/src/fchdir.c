/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_fchdir)
{
	int fd = open(TEST_DIR, O_RDONLY | O_DIRECTORY);

	zassert_true(fd >= 0);
	zassert_ok(fchdir(fd));

	IF_NOT_NATIVE_LIBC({
		char buf[64];

		/*
		 * The Zephyr working directory is lexical; the host may
		 * canonicalize symlinks, so only compare it there.
		 */
		zassert_not_null(getcwd(buf, sizeof(buf)));
		zassert_str_equal(buf, TEST_DIR);
	});

	zassert_ok(close(fd));

	/* fchdir() on a non-directory descriptor fails with ENOTDIR */
	fd = open(TEST_FILE, O_RDONLY);
	zassert_true(fd >= 0);
	errno = 0;
	zassert_equal(fchdir(fd), -1);
	zassert_equal(errno, ENOTDIR, "expected ENOTDIR got %d", errno);
	zassert_ok(close(fd));

	zassert_ok(chdir("/"));
}
