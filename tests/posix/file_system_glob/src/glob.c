/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_glob_tests.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

static void expect_paths(const glob_t *g, const char *const *paths, size_t n, size_t offs)
{
	zassert_equal((size_t)g->gl_pathc, n, "gl_pathc: %d, want %zu (first %s)", g->gl_pathc, n,
		      (g->gl_pathc > 0) ? g->gl_pathv[offs] : "");
	for (size_t i = 0; i < offs; i++) {
		zassert_is_null(g->gl_pathv[i]);
	}
	for (size_t i = 0; i < n; i++) {
		zassert_str_equal(g->gl_pathv[offs + i], paths[i], "[%zu]: %s, want %s", i,
				  g->gl_pathv[offs + i], paths[i]);
	}
	zassert_is_null(g->gl_pathv[offs + n]);
}

#define EXPECT_PATHS(g, ...)                                                                       \
	do {                                                                                       \
		const char *const paths_[] = {__VA_ARGS__};                                        \
		expect_paths((g), paths_, ARRAY_SIZE(paths_), 0);                                  \
	} while (0)

static void glob_matches(void)
{
	glob_t g;

	/* sorted, dot files excluded, a bracket expression in the pattern */
	zassert_ok(glob(TEST_ROOT "/*.txt", 0, NULL, &g));
	EXPECT_PATHS(&g, TEST_ROOT "/[ab].txt", TEST_ROOT "/a.txt", TEST_ROOT "/b.txt");
	globfree(&g);

	zassert_ok(glob(TEST_ROOT "/?.*", 0, NULL, &g));
	EXPECT_PATHS(&g, TEST_ROOT "/a.txt", TEST_ROOT "/b.txt", TEST_ROOT "/c.dat");
	globfree(&g);

	zassert_ok(glob(TEST_ROOT "/[ab].txt", 0, NULL, &g));
	EXPECT_PATHS(&g, TEST_ROOT "/a.txt", TEST_ROOT "/b.txt");
	globfree(&g);

	/* a leading period only matches explicitly */
	zassert_ok(glob(TEST_ROOT "/.h*", 0, NULL, &g));
	EXPECT_PATHS(&g, TEST_HIDDEN);
	globfree(&g);

	/* a literal pattern names one existing path */
	zassert_ok(glob(TEST_ROOT "/a.txt", 0, NULL, &g));
	EXPECT_PATHS(&g, TEST_ROOT "/a.txt");
	globfree(&g);

	/* a pattern segment in the middle of the path */
	zassert_ok(glob(TEST_ROOT "/*/?.txt", 0, NULL, &g));
	EXPECT_PATHS(&g, TEST_SUB "/x.txt", TEST_SUB "/y.txt");
	globfree(&g);

	zassert_ok(glob(TEST_SUB "/*", 0, NULL, &g));
	EXPECT_PATHS(&g, TEST_SUB "/x.txt", TEST_SUB "/y.txt");
	globfree(&g);
}

static void glob_relative(void)
{
	glob_t g;

	zassert_ok(chdir(TEST_ROOT));
	zassert_ok(glob("?.dat", 0, NULL, &g));
	EXPECT_PATHS(&g, "c.dat");
	globfree(&g);

	zassert_ok(glob("sub/*.txt", 0, NULL, &g));
	EXPECT_PATHS(&g, "sub/x.txt", "sub/y.txt");
	globfree(&g);
	zassert_ok(chdir("/"));
}

static void glob_nomatch(void)
{
	glob_t g;

	memset(&g, 0xff, sizeof(g));
	zassert_equal(glob(TEST_ROOT "/*.none", 0, NULL, &g), GLOB_NOMATCH);
	zassert_equal(g.gl_pathc, 0);
	globfree(&g);

	zassert_equal(glob(TEST_ROOT "/a.txt/*", 0, NULL, &g), GLOB_NOMATCH);
	globfree(&g);

	/* the pattern itself stands in for a missing match */
	zassert_ok(glob(TEST_ROOT "/*.none", GLOB_NOCHECK, NULL, &g));
	EXPECT_PATHS(&g, TEST_ROOT "/*.none");
	globfree(&g);
}

static void glob_mark(void)
{
	glob_t g;

	zassert_ok(glob(TEST_ROOT "/*", GLOB_MARK, NULL, &g));
	EXPECT_PATHS(&g, TEST_BRACKET, TEST_ROOT "/a.txt", TEST_ROOT "/b.txt", TEST_ROOT "/c.dat",
		     TEST_SUB "/");
	globfree(&g);

	zassert_ok(glob(TEST_SUB, GLOB_MARK, NULL, &g));
	EXPECT_PATHS(&g, TEST_SUB "/");
	globfree(&g);

	zassert_ok(glob(TEST_ROOT "/a.txt", GLOB_MARK, NULL, &g));
	EXPECT_PATHS(&g, TEST_ROOT "/a.txt");
	globfree(&g);

	/* a trailing slash matches directories only, and stays in the result */
	zassert_ok(glob(TEST_ROOT "/*/", 0, NULL, &g));
	EXPECT_PATHS(&g, TEST_SUB "/");
	globfree(&g);
}

static void glob_nosort(void)
{
	glob_t g;

	zassert_ok(glob(TEST_ROOT "/*.txt", GLOB_NOSORT, NULL, &g));
	zassert_equal(g.gl_pathc, 3);
	zassert_is_null(g.gl_pathv[3]);
	globfree(&g);
}

static void glob_escape(void)
{
	glob_t g;

	zassert_ok(glob(TEST_ROOT "/\\[ab\\].txt", 0, NULL, &g));
	EXPECT_PATHS(&g, TEST_BRACKET);
	globfree(&g);

	zassert_equal(glob(TEST_ROOT "/\\[ab\\].txt", GLOB_NOESCAPE, NULL, &g), GLOB_NOMATCH);
	globfree(&g);
}

static void glob_append(void)
{
	glob_t g;

	zassert_ok(glob(TEST_ROOT "/a.txt", 0, NULL, &g));
	zassert_ok(glob(TEST_ROOT "/*.dat", GLOB_APPEND, NULL, &g));
	zassert_ok(glob(TEST_SUB "/*", GLOB_APPEND, NULL, &g));
	EXPECT_PATHS(&g, TEST_ROOT "/a.txt", TEST_ROOT "/c.dat", TEST_SUB "/x.txt",
		     TEST_SUB "/y.txt");
	globfree(&g);

	/* an appended miss keeps what came before */
	zassert_ok(glob(TEST_ROOT "/a.txt", 0, NULL, &g));
	zassert_equal(glob(TEST_ROOT "/*.none", GLOB_APPEND, NULL, &g), GLOB_NOMATCH);
	EXPECT_PATHS(&g, TEST_ROOT "/a.txt");
	globfree(&g);
}

static void glob_doofs(void)
{
	static const char *const paths[] = {TEST_ROOT "/c.dat", TEST_SUB "/x.txt",
					    TEST_SUB "/y.txt"};
	glob_t g;

	memset(&g, 0, sizeof(g));
	g.gl_offs = 2;
	zassert_ok(glob(TEST_ROOT "/*.dat", GLOB_DOOFFS, NULL, &g));
	expect_paths(&g, paths, 1, 2);
	zassert_ok(glob(TEST_SUB "/*", GLOB_DOOFFS | GLOB_APPEND, NULL, &g));
	expect_paths(&g, paths, 3, 2);
	globfree(&g);
}

static ZTEST_BMEM int err_calls;
static ZTEST_BMEM int err_errno;
static ZTEST_BMEM char err_path[64];
static ZTEST_BMEM int err_ret;

static int errfunc(const char *epath, int eerrno)
{
	err_calls++;
	err_errno = eerrno;
	strncpy(err_path, epath, sizeof(err_path) - 1);
	return err_ret;
}

static void glob_err(void)
{
	glob_t g;

	/* an unreadable directory is skipped unless GLOB_ERR or errfunc says otherwise */
	zassert_equal(glob(TEST_NODIR "/*", 0, NULL, &g), GLOB_NOMATCH);
	globfree(&g);

	zassert_equal(glob(TEST_NODIR "/*", GLOB_ERR, NULL, &g), GLOB_ABORTED);
	globfree(&g);

	err_calls = 0;
	err_ret = 0;
	zassert_equal(glob(TEST_NODIR "/*", 0, errfunc, &g), GLOB_NOMATCH);
	zassert_equal(err_calls, 1);
	zassert_equal(err_errno, ENOENT);
	zassert_str_equal(err_path, TEST_NODIR);
	globfree(&g);

	err_calls = 0;
	err_ret = 1;
	zassert_equal(glob(TEST_NODIR "/*", 0, errfunc, &g), GLOB_ABORTED);
	zassert_equal(err_calls, 1);
	globfree(&g);
}

ZTEST_USER(posix_file_system_glob, test_glob)
{
	glob_matches();
	glob_relative();
	glob_nomatch();
	glob_mark();
	glob_nosort();
	glob_escape();
	glob_append();
	glob_doofs();
	glob_err();
}
