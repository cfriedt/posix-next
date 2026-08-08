/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <dirent.h>
#include <string.h>
#include <strings.h>

ZTEST_USER(posix_file_system, test_readdir_r)
{
	DIR *d = opendir(TEST_DIR);
	struct dirent entry;
	struct dirent *result;
	bool saw_sub = false;

	zassert_not_null(d);

	for (;;) {
		/* the host libc marks readdir_r() deprecated */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		zassert_ok(readdir_r(d, &entry, &result));
#pragma GCC diagnostic pop
		if (result == NULL) {
			break;
		}
		if (strcasecmp(result->d_name, "sub.txt") == 0) {
			saw_sub = true;
		}
	}
	zassert_true(saw_sub, "sub.txt not listed");

	zassert_ok(closedir(d));
}
