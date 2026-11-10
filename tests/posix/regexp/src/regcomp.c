/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <regex.h>

#include <zephyr/ztest.h>

static void regcomp_basic(void)
{
	regex_t re;

	zassert_ok(regcomp(&re, "a.c", 0));
	zassert_equal(re.re_nsub, 0);
	regfree(&re);

	/* BRE subexpressions are counted in re_nsub */
	zassert_ok(regcomp(&re, "\\(a*\\)b\\(c\\)", 0));
	zassert_equal(re.re_nsub, 2);
	regfree(&re);
}

static void regcomp_extended(void)
{
	regex_t re;

	zassert_ok(regcomp(&re, "(a|b)+c?", REG_EXTENDED));
	zassert_equal(re.re_nsub, 1);
	regfree(&re);

	zassert_ok(regcomp(&re, "^[[:alpha:]_][[:alnum:]_]*$", REG_EXTENDED));
	regfree(&re);
}

static void regcomp_nosub(void)
{
	regex_t re;

	zassert_ok(regcomp(&re, "(abc)", REG_EXTENDED | REG_NOSUB));
	zassert_ok(regexec(&re, "xxabcxx", 0, NULL, 0));
	regfree(&re);
}

static void regcomp_errors(void)
{
	regex_t re;

	zassert_equal(regcomp(&re, "[abc", REG_EXTENDED), REG_EBRACK);
	zassert_equal(regcomp(&re, "(abc", REG_EXTENDED), REG_EPAREN);
}

ZTEST(posix_regexp, test_regcomp)
{
	regcomp_basic();
	regcomp_extended();
	regcomp_nosub();
	regcomp_errors();
}
