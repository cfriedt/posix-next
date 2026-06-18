/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lib_ext, test_strdup)
{
	char *copy = strdup("posix");

	zexpect_not_null(copy);
	if (copy != NULL) {
		zexpect_str_equal(copy, "posix");
		free(copy);
	}
}
