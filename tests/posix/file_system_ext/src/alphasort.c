/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_ext_tests.h"

#include <dirent.h>
#include <string.h>

ZTEST_USER(posix_file_system_ext, test_alphasort)
{
	static const struct dirent first = {.d_name = "abc"};
	static const struct dirent second = {.d_name = "abd"};
	static const struct dirent longer = {.d_name = "abcd"};
	const struct dirent *d1 = &first;
	const struct dirent *d2 = &second;

	zassert_true(alphasort(&d1, &d2) < 0);
	zassert_true(alphasort(&d2, &d1) > 0);

	d2 = &first;
	zassert_equal(alphasort(&d1, &d2), 0);

	d2 = &longer;
	zassert_true(alphasort(&d1, &d2) < 0);
}
