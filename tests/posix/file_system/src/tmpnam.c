/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <stdio.h>

ZTEST(posix_file_system, test_tmpnam)
{
	char buf[L_tmpnam];
	char *p;

	p = tmpnam(NULL);
	zassert_not_null(p);

	zassert_equal_ptr(tmpnam(buf), buf);
	zassert_true(buf[0] != '\0');
}
