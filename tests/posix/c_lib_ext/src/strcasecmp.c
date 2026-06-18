/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <strings.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lib_ext, test_strcasecmp)
{
	zexpect_equal(strcasecmp("Hello", "hello"), 0);
	zexpect_equal(strcasecmp("abc", "abd"), -1);
	zexpect_equal(strcasecmp("abd", "abc"), 1);
	zexpect_true(strcasecmp("abc", "abcdef") < 0);
	zexpect_true(strcasecmp("abcdef", "abc") > 0);
}
