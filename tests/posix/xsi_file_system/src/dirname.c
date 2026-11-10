/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <libgen.h>
#include <string.h>

ZTEST_USER(xsi_file_system, test_dirname)
{
	static const struct {
		const char *in;
		const char *out;
	} cases[] = {
		{"usr", "."},  {"usr/", "."},            {"", "."},        {"/", "/"},
		{"///", "/"},  {"//usr//lib//", "//usr"}, {"/usr/", "/"},   {"/usr/lib", "/usr"},
	};
	char buf[32];

	ARRAY_FOR_EACH(cases, i) {
		strcpy(buf, cases[i].in);
		zassert_str_equal(dirname(buf), cases[i].out, "dirname(\"%s\")", cases[i].in);
	}
	zassert_str_equal(dirname(NULL), ".");
}
