/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_mkstemp)
{
	char tmpl[] = TEST_ROOT "/XXXXXX";
	char bad[] = "noXtemplate";
	struct stat st;
	int fd;

	fd = mkstemp(tmpl);
	zassert_true(fd >= 0, "mkstemp failed: %d", errno);
	zassert_true(strstr(tmpl, "XXXXXX") == NULL, "template not filled");
	zassert_ok(fstat(fd, &st));
	zassert_true(S_ISREG(st.st_mode));
	zassert_ok(close(fd));
	zassert_ok(unlink(tmpl));

	errno = 0;
	zassert_equal(mkstemp(bad), -1);
	zassert_equal(errno, EINVAL);
}
