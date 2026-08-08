/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <dirent.h>

ZTEST_USER(posix_file_system, test_rewinddir)
{
	DIR *d = opendir(TEST_DIR);
	int n = 0;

	zassert_not_null(d);
	while (readdir(d) != NULL) {
		n++;
	}
	zassert_true(n >= 1);

	rewinddir(d);
	zassert_not_null(readdir(d), "rewound stream lists entries again");

	zassert_ok(closedir(d));
}
