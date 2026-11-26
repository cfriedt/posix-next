/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/sys/zvfs_fs.h>

ZTEST_USER(xsi_file_system, test_tempnam)
{
	char *a = tempnam(TEST_DIR, "pfx");
	char *b = tempnam(TEST_DIR, "pfx");
	char *c;

	/* in the directory, with the prefix, naming nothing that exists */
	zassert_not_null(a);
	zassert_not_null(b);
	zassert_equal(strncmp(a, TEST_DIR "/pfx", strlen(TEST_DIR "/pfx")), 0, "%s", a);
	zassert_equal(strlen(a), strlen(TEST_DIR "/pfx") + 6);
	errno = 0;
	zassert_equal(zvfs_access(a, ZVFS_F_OK), -1);
	zassert_equal(errno, ENOENT);
	zassert_true(strcmp(a, b) != 0, "two names: %s", a);
	free(a);
	free(b);

	/* at most five characters of the prefix; no prefix at all */
	c = tempnam(TEST_DIR, "abcdefgh");
	zassert_not_null(c);
	zassert_equal(strncmp(c, TEST_DIR "/abcde", strlen(TEST_DIR "/abcde")), 0, "%s", c);
	zassert_equal(strlen(c), strlen(TEST_DIR "/abcde") + 6);
	free(c);
	c = tempnam(TEST_DIR, NULL);
	zassert_not_null(c);
	zassert_equal(strlen(c), strlen(TEST_DIR "/") + 6);
	free(c);

	/* no usable directory given: P_tmpdir */
	c = tempnam(NULL, "t");
	zassert_not_null(c);
	zassert_equal(strncmp(c, P_tmpdir "/t", strlen(P_tmpdir "/t")), 0, "%s", c);
	free(c);
	c = tempnam(TEST_NOENT, "t");
	zassert_not_null(c);
	zassert_equal(strncmp(c, P_tmpdir "/t", strlen(P_tmpdir "/t")), 0, "%s", c);
	free(c);
}
