/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <time.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lang_support_r, test_asctime_r)
{
	char buf[26] = { 0 };
	struct tm tp = {
		.tm_sec = 10,   /* Seconds */
		.tm_min = 30,   /* Minutes */
		.tm_hour = 14,  /* Hour (24-hour format) */
		.tm_wday = 5,   /* Day of the week (0-6, 0 = Sun) */
		.tm_mday = 1,   /* Day of the month */
		.tm_mon = 5,    /* Month (0-11, January = 0) */
		.tm_year = 124, /* Year (current year - 1900) */
	};

	zassert_not_null(asctime_r(&tp, buf));
	zassert_ok(strncmp("Fri Jun  1 14:30:10 2024\n", buf, sizeof(buf)));
}
