/*
 * Copyright (c) 2017 Intel Corporation
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

struct time_r_test_data {
	time_t ux;
	struct tm tm;
};

static void check_gmtime_r(const struct time_r_test_data *tp)
{
	struct tm result = { .tm_isdst = 1234 };
	struct tm *ret;
	long long ux = (long long)tp->ux;

	ret = gmtime_r(&tp->ux, &result);
	zassert_equal(&result, ret, "gmtime_r returned unexpected pointer");

	zassert_equal(tp->tm.tm_sec, result.tm_sec, "sec mismatch for ux %lld", ux);
	zassert_equal(tp->tm.tm_min, result.tm_min, "min mismatch for ux %lld", ux);
	zassert_equal(tp->tm.tm_hour, result.tm_hour, "hour mismatch for ux %lld", ux);
	zassert_equal(tp->tm.tm_mday, result.tm_mday, "mday mismatch for ux %lld", ux);
	zassert_equal(tp->tm.tm_mon, result.tm_mon, "mon mismatch for ux %lld", ux);
	zassert_equal(tp->tm.tm_year, result.tm_year, "year mismatch for ux %lld", ux);
	zassert_equal(tp->tm.tm_wday, result.tm_wday, "wday mismatch for ux %lld", ux);
	zassert_equal(tp->tm.tm_yday, result.tm_yday, "yday mismatch for ux %lld", ux);
}

ZTEST(posix_c_lang_support_r, test_gmtime_r)
{
	static const struct time_r_test_data tests[] = {
		{
			.ux = 0,
			.tm = {
				.tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1,
				.tm_mon = 0, .tm_year = 70, .tm_wday = 4, .tm_yday = 0,
			},
		},
		{
			.ux = -1,
			.tm = {
				.tm_sec = 59, .tm_min = 59, .tm_hour = 23, .tm_mday = 31,
				.tm_mon = 11, .tm_year = 69, .tm_wday = 3, .tm_yday = 364,
			},
		},
		{
			.ux = 1561994005,
			.tm = {
				.tm_sec = 25, .tm_min = 13, .tm_hour = 15, .tm_mday = 1,
				.tm_mon = 6, .tm_year = 119, .tm_wday = 1, .tm_yday = 181,
			},
		},
		{
			.ux = 951868800,
			.tm = {
				.tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1,
				.tm_mon = 2, .tm_year = 100, .tm_wday = 3, .tm_yday = 60,
			},
		},
	};

	time_t tests3 = (time_t)-214748364800;
	time_t tests4 = 951868800;
	struct tm tp;

	tp.tm_wday = -5;
	zassert_not_null(gmtime_r(&tests3, &tp), "gmtime_r failed");
	zassert_not_null(gmtime_r(&tests4, &tp), "gmtime_r failed");

	for (size_t i = 0; i < ARRAY_SIZE(tests); ++i) {
		check_gmtime_r(&tests[i]);
	}
}
