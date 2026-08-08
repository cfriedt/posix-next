/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

ZTEST_USER(posix_file_system, test_readdir)
{
	DIR *d = opendir(TEST_DIR);
	struct dirent *e;
	bool saw_sub = false;

	zassert_not_null(d);

	errno = 0xbeef;
	while ((e = readdir(d)) != NULL) {
		if (strcasecmp(e->d_name, "sub.txt") == 0) {
			saw_sub = true;
		}
	}
	/* end of directory leaves errno untouched */
	zassert_equal(errno, 0xbeef);
	zassert_true(saw_sub, "sub.txt not listed");

	zassert_ok(closedir(d));
}
