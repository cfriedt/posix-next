/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "symbolic_links_tests.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_symbolic_links, test_lstat)
{
	struct stat st;

	/* without a link in the way, lstat() is stat() */
	zassert_ok(lstat(TEST_FILE, &st));
	zassert_true(S_ISREG(st.st_mode));
	zassert_equal(st.st_size, strlen(TEST_CONTENT));
	zassert_ok(lstat(TEST_ROOT, &st));
	zassert_true(S_ISDIR(st.st_mode));

	/* the link itself, not the file behind it */
	zassert_ok(symlink("file.txt", TEST_LINK));
	zassert_ok(lstat(TEST_LINK, &st));
	zassert_true(S_ISLNK(st.st_mode));
	zassert_equal(st.st_size, strlen("file.txt"));
	zassert_ok(stat(TEST_LINK, &st));
	zassert_true(S_ISREG(st.st_mode));
	zassert_equal(st.st_size, strlen(TEST_CONTENT));
	zassert_ok(unlink(TEST_LINK));

	errno = 0;
	zassert_equal(lstat(TEST_NOENT, &st), -1);
	zassert_equal(errno, ENOENT);
}
