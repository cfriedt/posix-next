/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lang_support_r, test_ctime_r)
{
	char buf[26] = { 0 };
	time_t test1 = 1718260000;

#ifdef CONFIG_NATIVE_LIBC
	setenv("TZ", "UTC", 1);
#endif
	zassert_not_null(ctime_r(&test1, buf));
	zassert_ok(strncmp("Thu Jun 13 06:26:40 2024\n", buf, sizeof(buf)));
}
