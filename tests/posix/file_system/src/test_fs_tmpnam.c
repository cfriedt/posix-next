/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/ztest.h>

#include "test_fs.h"

ZTEST(posix_fs_test, test_tmpnam)
{
	char *name;

	name = tmpnam(NULL);
	if (name == NULL) {
		TC_PRINT("tmpnam() not supported in this configuration\n");
		ztest_test_skip();
	}
	zassert_not_null(name);
}
