/*
 * Copyright (c) 2021 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* qsort_r() is part of the 2024 POSIX standard (Issue 8) */
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 202405L

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/posix/posix_stdlib.h>
#include <zephyr/ztest.h>

struct test_qsort_case {
	const char *name;
	size_t len;
	const int *input;
	const int *expect;
	size_t base_index;
	size_t nmemb;
};

extern const struct test_qsort_case test_qsort_input[];
extern const size_t test_qsort_input_len;

static int compare_ints_with_boolp_arg(const void *a, const void *b, void *argp)
{
	int aa = *(const int *)a;
	int bb = *(const int *)b;

	*(bool *)argp = true;

	return (aa > bb) - (aa < bb);
}

ZTEST(posix_c_lang_support_r, test_qsort_r)
{
	static int actual_int[93];
	bool arg;

	for (size_t i = 0; i < test_qsort_input_len; i++) {
		const struct test_qsort_case *tc = &test_qsort_input[i];

		memcpy(actual_int, tc->input, tc->len * sizeof(int));
		arg = false;
		qsort_r(&actual_int[tc->base_index], tc->nmemb, sizeof(int),
			compare_ints_with_boolp_arg, &arg);
		zassert_mem_equal(actual_int, tc->expect, tc->len * sizeof(int), "%s", tc->name);
		if (tc->nmemb >= 2) {
			zassert_true(arg, "case %s arg not modified", tc->name);
		}
	}
}
