/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <regex.h>

#include <zephyr/ztest.h>

static void regfree_releases(void)
{
	regex_t re;

	/* repeated compile/free cycles must not exhaust the heap */
	for (int i = 0; i < 32; i++) {
		zassert_ok(regcomp(&re, "(a|b)*c[[:digit:]]?", REG_EXTENDED));
		regfree(&re);
	}
}

static void regfree_recompile(void)
{
	regex_t re;

	zassert_ok(regcomp(&re, "first", 0));
	regfree(&re);

	/* a freed regex_t is reusable for a fresh compile */
	zassert_ok(regcomp(&re, "second", 0));
	zassert_ok(regexec(&re, "the second one", 0, NULL, 0));
	regfree(&re);
}

ZTEST(posix_regexp, test_regfree)
{
	regfree_releases();
	regfree_recompile();
}
