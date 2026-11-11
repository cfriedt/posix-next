/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/sys/zvfs_fs.h>

#define expect_path(in, out)                                                                       \
	do {                                                                                       \
		char *res = realpath(in, buf);                                                     \
		zassert_not_null(res, "realpath(\"%s\"): errno %d", in, errno);                    \
		zassert_str_equal(res, out, "realpath(\"%s\")", in);                               \
	} while (0)

ZTEST_USER(xsi_file_system, test_realpath)
{
	char buf[PATH_MAX];
	char *r;

	/* dot, dot-dot and repeated slashes */
	expect_path(TEST_ROOT "/dir/../dir/./sub.txt", TEST_SUB);
	expect_path(TEST_ROOT "//dir//", TEST_DIR);
	expect_path(FS_TMPDIR "/..", "/");
	expect_path("/", "/");

	/* a symbolic link on the way */
	expect_path(TEST_LINK, TEST_SUB);

	/* relative to the working directory */
	zassert_ok(zvfs_chdir(TEST_DIR));
	expect_path("sub.txt", TEST_SUB);
	expect_path("..", TEST_ROOT);
	expect_path(".", TEST_DIR);
	zassert_ok(zvfs_chdir("/"));

	/* no buffer: one is allocated */
	r = realpath(TEST_FILE, NULL);
	zassert_not_null(r);
	zassert_str_equal(r, TEST_FILE);
	free(r);

	errno = 0;
	zassert_is_null(realpath(TEST_NOENT, buf));
	zassert_equal(errno, ENOENT);
	errno = 0;
	zassert_is_null(realpath(TEST_DANGLING, buf));
	zassert_equal(errno, ENOENT);
	errno = 0;
	zassert_is_null(realpath(TEST_FILE "/x", buf));
	zassert_equal(errno, ENOTDIR);
	errno = 0;
	zassert_is_null(realpath("", buf));
	zassert_equal(errno, ENOENT);
	errno = 0;
	zassert_is_null(realpath(NULL, buf));
	zassert_equal(errno, EINVAL);
}
