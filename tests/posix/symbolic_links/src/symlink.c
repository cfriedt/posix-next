/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "symbolic_links_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_symbolic_links, test_symlink)
{
	struct stat st;
	char buf[sizeof(TEST_CONTENT)];
	int fd;

	zassert_ok(symlink(TEST_FILE, TEST_LINK));
	zassert_ok(lstat(TEST_LINK, &st));
	zassert_true(S_ISLNK(st.st_mode));
	zassert_ok(stat(TEST_LINK, &st));
	zassert_true(S_ISREG(st.st_mode));

	/* following the link reaches the file */
	fd = open(TEST_LINK, O_RDONLY);
	zassert_true(fd >= 0, "open(link) failed: %d", errno);
	zassert_equal(read(fd, buf, sizeof(buf) - 1), strlen(TEST_CONTENT));
	zassert_ok(close(fd));

	errno = 0;
	zassert_equal(symlink(TEST_FILE, TEST_LINK), -1);
	zassert_equal(errno, EEXIST);

	/* removing the link leaves the file */
	zassert_ok(unlink(TEST_LINK));
	zassert_ok(stat(TEST_FILE, &st));
	zassert_equal(st.st_size, strlen(TEST_CONTENT));

	/* the target need not exist */
	zassert_ok(symlink("nope", TEST_DANGLE));
	zassert_ok(lstat(TEST_DANGLE, &st));
	zassert_true(S_ISLNK(st.st_mode));
	errno = 0;
	zassert_equal(stat(TEST_DANGLE, &st), -1);
	zassert_equal(errno, ENOENT);
	zassert_ok(unlink(TEST_DANGLE));

	errno = 0;
	zassert_equal(symlink(TEST_FILE, TEST_NOENT "/link"), -1);
	zassert_equal(errno, ENOENT);
}
