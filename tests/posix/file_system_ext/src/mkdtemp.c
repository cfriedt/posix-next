/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_ext_tests.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_ext, test_mkdtemp)
{
	char bad[] = TEST_ROOT "/dNOTEMP";
	char tmpl[] = TEST_ROOT "/dXXXXXX";
	struct stat st;

	errno = 0;
	zassert_is_null(mkdtemp(bad));
	zassert_equal(errno, EINVAL);

	zassert_equal(mkdtemp(tmpl), tmpl);
	zassert_true(strncmp(tmpl, TEST_ROOT "/d", strlen(TEST_ROOT "/d")) == 0);
	zassert_true(strcmp(tmpl, TEST_ROOT "/dXXXXXX") != 0);

	zassert_ok(stat(tmpl, &st));
	zassert_true(S_ISDIR(st.st_mode));

	zassert_ok(rmdir(tmpl));
}
