/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <regex.h>

#include <zephyr/ztest.h>

static void regexec_literal(void)
{
	regex_t re;

	zassert_ok(regcomp(&re, "hello", 0));
	zassert_ok(regexec(&re, "say hello world", 0, NULL, 0));
	zassert_equal(regexec(&re, "goodbye", 0, NULL, 0), REG_NOMATCH);
	regfree(&re);
}

static void regexec_subexpressions(void)
{
	regex_t re;
	regmatch_t pmatch[3];

	zassert_ok(regcomp(&re, "([a-z]+)([0-9]+)", REG_EXTENDED));
	zassert_ok(regexec(&re, "__abc123def", ARRAY_SIZE(pmatch), pmatch, 0));

	/* leftmost-longest: the whole match and both captures */
	zassert_equal(pmatch[0].rm_so, 2);
	zassert_equal(pmatch[0].rm_eo, 8);
	zassert_equal(pmatch[1].rm_so, 2);
	zassert_equal(pmatch[1].rm_eo, 5);
	zassert_equal(pmatch[2].rm_so, 5);
	zassert_equal(pmatch[2].rm_eo, 8);
	regfree(&re);
}

static void regexec_icase(void)
{
	regex_t re;

	zassert_ok(regcomp(&re, "hello", REG_ICASE));
	zassert_ok(regexec(&re, "HeLLo", 0, NULL, 0));
	regfree(&re);
}

static void regexec_newline(void)
{
	regex_t re;

	zassert_ok(regcomp(&re, "^b", REG_EXTENDED));
	zassert_equal(regexec(&re, "a\nb", 0, NULL, 0), REG_NOMATCH);
	regfree(&re);

	zassert_ok(regcomp(&re, "^b", REG_EXTENDED | REG_NEWLINE));
	zassert_ok(regexec(&re, "a\nb", 0, NULL, 0));
	regfree(&re);
}

static void regexec_anchoring_eflags(void)
{
	regex_t re;

	zassert_ok(regcomp(&re, "^abc", REG_EXTENDED));
	zassert_ok(regexec(&re, "abc", 0, NULL, 0));
	zassert_equal(regexec(&re, "abc", 0, NULL, REG_NOTBOL), REG_NOMATCH);
	regfree(&re);

	zassert_ok(regcomp(&re, "abc$", REG_EXTENDED));
	zassert_ok(regexec(&re, "abc", 0, NULL, 0));
	zassert_equal(regexec(&re, "abc", 0, NULL, REG_NOTEOL), REG_NOMATCH);
	regfree(&re);
}

ZTEST(posix_regexp, test_regexec)
{
	regexec_literal();
	regexec_subexpressions();
	regexec_icase();
	regexec_newline();
	regexec_anchoring_eflags();
}
