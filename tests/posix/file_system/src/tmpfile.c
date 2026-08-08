/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <stdio.h>

ZTEST(posix_file_system, test_tmpfile)
{
	FILE *fp = tmpfile();

	if (fp == NULL) {
		/* requires a mounted /tmp; skip where unavailable */
		ztest_test_skip();
	}

	zassert_true(fwrite("x", 1, 1, fp) == 1);
	zassert_ok(fclose(fp));
}
