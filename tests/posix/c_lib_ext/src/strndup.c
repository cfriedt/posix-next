/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lib_ext, test_strndup)
{
	char *copy = strndup("posix-next", 7);

	zexpect_not_null(copy);
	if (copy != NULL) {
		zexpect_str_equal(copy, "posix-n");
		free(copy);
	}

	copy = strndup("hello", 4);
	zexpect_not_null(copy);
	if (copy != NULL) {
		zexpect_str_equal(copy, "hell");
		free(copy);
	}

	copy = strndup("posix", 8);
	zexpect_not_null(copy);
	if (copy != NULL) {
		zexpect_str_equal(copy, "posix");
		free(copy);
	}

	copy = strndup("abc", 0);
	zexpect_not_null(copy);
	if (copy != NULL) {
		zexpect_str_equal(copy, "");
		free(copy);
	}
}
