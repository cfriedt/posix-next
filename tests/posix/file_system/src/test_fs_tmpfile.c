/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>

#include <zephyr/ztest.h>

#include "test_fs.h"

ZTEST(posix_fs_test, test_tmpfile)
{
	FILE *fp;

	fp = tmpfile();
	if (fp == NULL) {
		TC_PRINT("tmpfile() not supported in this configuration (errno=%d)\n", errno);
		ztest_test_skip();
	}
	zassert_ok(fclose(fp));
}
