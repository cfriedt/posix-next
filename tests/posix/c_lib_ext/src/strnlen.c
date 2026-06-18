/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lib_ext, test_strnlen)
{
	zexpect_equal(strnlen("hello", 3), 3);
	zexpect_equal(strnlen("hello", 8), 5);
	zexpect_equal(strnlen("", 4), 0);
}
