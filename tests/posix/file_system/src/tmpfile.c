/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

ZTEST(posix_file_system, test_tmpfile)
{
	char probe[L_tmpnam];
	char expected[L_tmpnam];
	unsigned int counter;
	struct stat sb;
	FILE *fp;

	/* tmpnam() names are "/tmp/NN", a two-digit counter: predict tmpfile()'s pick */
	if ((tmpnam(probe) == NULL) || (strlen(probe) != 7) || (probe[5] < '0') ||
	    (probe[5] > '9') || (probe[6] < '0') || (probe[6] > '9')) {
		ztest_test_skip();
	}
	counter = (unsigned int)((probe[5] - '0') * 10 + (probe[6] - '0'));
	(void)snprintf(expected, sizeof(expected), "/tmp/%02u", counter + 1);

	fp = tmpfile();
	if (fp == NULL) {
		/* requires a mounted /tmp; skip where unavailable */
		ztest_test_skip();
	}

	zassert_true(fwrite("x", 1, 1, fp) == 1);
	zassert_ok(stat(expected, &sb), "the stream is not backed by %s", expected);
	zassert_ok(fclose(fp));

	/* the file is removed when the stream is closed (ISO C) */
	errno = 0;
	zassert_equal(-1, stat(expected, &sb), "%s survived fclose()", expected);
	zassert_equal(ENOENT, errno);
}
