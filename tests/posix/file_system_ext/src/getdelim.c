/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_ext_tests.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void getdelim_invalid_args(void)
{
	FILE *fp = fopen(TEST_FIELDS, "r");
	char *line = NULL;
	size_t n = 0;

	zassert_not_null(fp);

	errno = 0;
	zassert_equal(getdelim(NULL, &n, ':', fp), -1);
	zassert_equal(errno, EINVAL);

	/* no errno check: glibc returns -1 for a null n without setting it */
	zassert_equal(getdelim(&line, NULL, ':', fp), -1);

	zassert_ok(fclose(fp));
}

/*
 * POSIX: a null *lineptr requests allocation and *n is only meaningful for a
 * caller-supplied buffer. A stale size with a null buffer must be ignored
 * (callers like toybox pass an uninitialized size with a null buffer).
 */
static void getdelim_null_buffer_stale_size(void)
{
	FILE *fp = fopen(TEST_LINES, "r");
	char *line = NULL;
	size_t n = SIZE_MAX;
	ssize_t len;

	zassert_not_null(fp);

	len = getdelim(&line, &n, '\n', fp);
	zassert_equal(len, strlen(TEST_LINE1));
	zassert_not_null(line);
	zassert_true(n > strlen(TEST_LINE1));
	zassert_mem_equal(line, TEST_LINE1, strlen(TEST_LINE1) + 1);

	free(line);
	zassert_ok(fclose(fp));
}

static void getdelim_grows_and_reuses(void)
{
	FILE *fp = fopen(TEST_LINES, "r");
	char *line = malloc(4);
	size_t n = 4;
	ssize_t len;

	zassert_not_null(fp);
	zassert_not_null(line);

	len = getdelim(&line, &n, '\n', fp);
	zassert_equal(len, strlen(TEST_LINE1));
	zassert_mem_equal(line, TEST_LINE1, strlen(TEST_LINE1) + 1);

	/* the second line is longer than any initial allocation */
	len = getdelim(&line, &n, '\n', fp);
	zassert_equal(len, strlen(TEST_LINE2));
	zassert_mem_equal(line, TEST_LINE2, strlen(TEST_LINE2) + 1);
	zassert_true(n > strlen(TEST_LINE2));

	len = getdelim(&line, &n, '\n', fp);
	zassert_equal(len, strlen(TEST_LINE3));

	/* the tail has no delimiter: its length is still returned */
	len = getdelim(&line, &n, '\n', fp);
	zassert_equal(len, strlen(TEST_TAIL));
	zassert_mem_equal(line, TEST_TAIL, strlen(TEST_TAIL) + 1);

	/* nothing left */
	zassert_equal(getdelim(&line, &n, '\n', fp), -1);

	free(line);
	zassert_ok(fclose(fp));
}

static void getdelim_custom_delimiter(void)
{
	static const char *const fields[] = {"one:", "two:", ":", "three"};
	FILE *fp = fopen(TEST_FIELDS, "r");
	char *line = NULL;
	size_t n = 0;

	zassert_not_null(fp);

	for (size_t i = 0; i < ARRAY_SIZE(fields); i++) {
		zassert_equal(getdelim(&line, &n, ':', fp), strlen(fields[i]));
		zassert_mem_equal(line, fields[i], strlen(fields[i]) + 1);
	}
	zassert_equal(getdelim(&line, &n, ':', fp), -1);

	free(line);
	zassert_ok(fclose(fp));
}

static void getdelim_empty_file(void)
{
	FILE *fp = fopen(TEST_EMPTY, "r");
	char *line = NULL;
	size_t n = 0;

	zassert_not_null(fp);

	zassert_equal(getdelim(&line, &n, '\n', fp), -1);

	free(line);
	zassert_ok(fclose(fp));
}

ZTEST_USER(posix_file_system_ext, test_getdelim)
{
	getdelim_invalid_args();
	getdelim_null_buffer_stale_size();
	getdelim_grows_and_reuses();
	getdelim_custom_delimiter();
	getdelim_empty_file();
}
