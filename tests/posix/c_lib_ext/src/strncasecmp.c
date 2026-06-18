/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <strings.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lib_ext, test_strncasecmp)
{
	zexpect_equal(strncasecmp("Hello", "hello", 5), 0);
	zexpect_equal(strncasecmp("abc", "abd", 2), 0);
	zexpect_equal(strncasecmp("abc", "abd", 3), -1);
	zexpect_true(strncasecmp("ab", "abc", 3) < 0);
	zexpect_true(strncasecmp("abc", "ab", 3) > 0);
}
