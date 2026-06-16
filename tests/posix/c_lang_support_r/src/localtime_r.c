/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
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
#ifdef CONFIG_NATIVE_LIBC
	setenv("TZ", "UTC", 1);
#endif
	zassert_not_null(localtime_r(&ux, &local), "localtime_r failed");
	zassert_equal(gm.tm_sec, local.tm_sec, "sec mismatch");
	zassert_equal(gm.tm_min, local.tm_min, "min mismatch");
	zassert_equal(gm.tm_hour, local.tm_hour, "hour mismatch");
	zassert_equal(gm.tm_mday, local.tm_mday, "mday mismatch");
	zassert_equal(gm.tm_mon, local.tm_mon, "mon mismatch");
	zassert_equal(gm.tm_year, local.tm_year, "year mismatch");
	zassert_equal(gm.tm_wday, local.tm_wday, "wday mismatch");
	zassert_equal(gm.tm_yday, local.tm_yday, "yday mismatch");
	zassert_equal(gm.tm_isdst, local.tm_isdst, "isdst mismatch");
}
