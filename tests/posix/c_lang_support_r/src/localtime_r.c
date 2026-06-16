/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <time.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lang_support_r, test_localtime_r)
{
	time_t tests3 = (time_t)-214748364800;
	time_t tests4 = 951868800;
	time_t ux = 1718260000;
	struct tm tp;
	struct tm gm = { 0 };
	struct tm local = { 0 };

	tp.tm_wday = -5;
	zassert_not_null(localtime_r(&tests3, &tp), "localtime_r failed");
	zassert_not_null(localtime_r(&tests4, &tp), "localtime_r failed");

	zassert_not_null(gmtime_r(&ux, &gm), "gmtime_r failed");
	zassert_not_null(localtime_r(&ux, &local), "localtime_r failed");
	zassert_mem_equal(&gm, &local, sizeof(struct tm),
			  "localtime_r should match gmtime_r in UTC mode");
}
