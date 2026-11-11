/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_glob_tests.h"

#include <string.h>

ZTEST_USER(posix_file_system_glob, test_globfree)
{
	glob_t g;

	/* nothing to free */
	memset(&g, 0, sizeof(g));
	globfree(&g);

	zassert_ok(glob(TEST_ROOT "/*.txt", 0, NULL, &g));
	zassert_equal(g.gl_pathc, 3);
	globfree(&g);
	zassert_is_null(g.gl_pathv);
	/* already freed */
	globfree(&g);

	/* reserved slots are released with the matches */
	memset(&g, 0, sizeof(g));
	g.gl_offs = 2;
	zassert_ok(glob(TEST_ROOT "/*.txt", GLOB_DOOFFS, NULL, &g));
	zassert_ok(glob(TEST_ROOT "/*.dat", GLOB_DOOFFS | GLOB_APPEND, NULL, &g));
	zassert_equal(g.gl_pathc, 4);
	globfree(&g);
	zassert_is_null(g.gl_pathv);
}
