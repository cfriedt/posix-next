/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lib_ext, test_stpncpy)
{
	/* truncated copy: no terminating NUL within n bytes */
	char short_buf[4];

	memset(short_buf, 'x', sizeof(short_buf));
	zexpect_equal_ptr(stpncpy(short_buf, "abcdef", sizeof(short_buf)),
			  &short_buf[sizeof(short_buf)]);
	zexpect_mem_equal(short_buf, "abc", 3);
}
