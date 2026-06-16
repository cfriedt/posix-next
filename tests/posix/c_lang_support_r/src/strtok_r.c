/*
 * Copyright (c) 2017 Intel Corporation
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <string.h>

#include <zephyr/ztest.h>

struct strtok_test_case {
	const char *input;
	const char *sep;
	int tlen;
	const char *const *toks;
	bool expect;
};

extern const struct strtok_test_case strtok_test_cases[];
extern const size_t strtok_test_cases_len;

static void run_strtok_r_case(const struct strtok_test_case *tc)
{
	int len = 0;
	char *state, *tok, buf[64 + 1] = { 0 };

	strncpy(buf, tc->input, 64);

	tok = strtok_r(buf, tc->sep, &state);
	while (tok && len < tc->tlen) {
		if (strcmp(tok, tc->toks[len]) != 0) {
			break;
		}
		tok = strtok_r(NULL, tc->sep, &state);
		len++;
	}
	if (tc->expect) {
		zassert_equal(len, tc->tlen, "strtok_r error '%s' / '%s'", tc->input, tc->sep);
	} else {
		zassert_not_equal(len, tc->tlen, "strtok_r error '%s' / '%s'", tc->input, tc->sep);
	}
}

ZTEST(posix_c_lang_support_r, test_strtok_r)
{
	for (size_t i = 0; i < strtok_test_cases_len; i++) {
		run_strtok_r_case(&strtok_test_cases[i]);
	}
}
