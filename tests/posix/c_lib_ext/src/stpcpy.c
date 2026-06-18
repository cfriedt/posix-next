/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lib_ext, test_stpcpy)
{
	char buf[16];

	zexpect_equal_ptr(stpcpy(buf, "hello"), &buf[5]);
	zexpect_str_equal(buf, "hello");
}
