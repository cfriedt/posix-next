/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_ext_tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ZTEST_USER(posix_file_system_ext, test_getline)
{
	static const char *const lines[] = {TEST_LINE1, TEST_LINE2, TEST_LINE3, TEST_TAIL};
	FILE *fp = fopen(TEST_LINES, "r");
	char *line = NULL;
	size_t n = 0;

	zassert_not_null(fp);

	for (size_t i = 0; i < ARRAY_SIZE(lines); i++) {
		zassert_equal(getline(&line, &n, fp), strlen(lines[i]));
		zassert_mem_equal(line, lines[i], strlen(lines[i]) + 1);
	}
	zassert_equal(getline(&line, &n, fp), -1);

	free(line);
	zassert_ok(fclose(fp));
}
