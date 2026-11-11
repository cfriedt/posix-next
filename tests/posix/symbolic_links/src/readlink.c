/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "symbolic_links_tests.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

ZTEST_USER(posix_symbolic_links, test_readlink)
{
	char buf[32];
	ssize_t len;

	zassert_ok(symlink("file.txt", TEST_LINK));

	len = readlink(TEST_LINK, buf, sizeof(buf));
	zassert_equal(len, strlen("file.txt"), "readlink: %zd (errno %d)", len, errno);
	zassert_mem_equal(buf, "file.txt", len);

	/* a short buffer truncates, without a terminator */
	memset(buf, 'x', sizeof(buf));
	len = readlink(TEST_LINK, buf, 4);
	zassert_equal(len, 4, "readlink: %zd (errno %d)", len, errno);
	zassert_mem_equal(buf, "filex", 5);

	errno = 0;
	zassert_equal(readlink(TEST_FILE, buf, sizeof(buf)), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(readlink(TEST_NOENT, buf, sizeof(buf)), -1);
	zassert_equal(errno, ENOENT);

	zassert_ok(unlink(TEST_LINK));
}
