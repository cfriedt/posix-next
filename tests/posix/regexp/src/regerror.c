/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <regex.h>
#include <string.h>

#include <zephyr/ztest.h>

static void regerror_reports_size(void)
{
	int err;
	size_t needed;
	regex_t re;
	char buf[128];

	err = regcomp(&re, "(abc", REG_EXTENDED);
	zassert_equal(err, REG_EPAREN);

	/* with errbuf_size 0, the return value is the size needed */
	needed = regerror(err, &re, NULL, 0);
	zassert_true(needed > 1);
	zassert_true(needed <= sizeof(buf));

	zassert_equal(regerror(err, &re, buf, sizeof(buf)), needed);
	zassert_equal(strlen(buf) + 1, needed);
}

static void regerror_truncates(void)
{
	regex_t re;
	char small[4];

	zassert_ok(regcomp(&re, "abc", 0));

	/* a too-small buffer yields a null-terminated truncated message */
	zassert_true(regerror(REG_NOMATCH, &re, small, sizeof(small)) > sizeof(small));
	zassert_equal(strlen(small), sizeof(small) - 1);
	regfree(&re);
}

ZTEST(posix_regexp, test_regerror)
{
	regerror_reports_size();
	regerror_truncates();
}
