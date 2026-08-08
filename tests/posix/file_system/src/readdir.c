/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/* d_type and the DT_* values are an extension glibc hides behind _DEFAULT_SOURCE */
#define _DEFAULT_SOURCE 1

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
			zassert_equal(e->d_type, DT_REG, "d_type of a file: %d", e->d_type);
		}
	}
	/* end of directory leaves errno untouched */
	zassert_equal(errno, 0xbeef);
	zassert_true(saw_sub, "sub.txt not listed");

	zassert_ok(closedir(d));

	/* the type of a directory entry is reported too */
	d = opendir(TEST_ROOT);
	zassert_not_null(d);
	saw_sub = false;
	while ((e = readdir(d)) != NULL) {
		if (strcasecmp(e->d_name, "dir") == 0) {
			saw_sub = true;
			zassert_equal(e->d_type, DT_DIR, "d_type of a directory: %d", e->d_type);
		}
	}
	zassert_true(saw_sub, "dir not listed");
	zassert_ok(closedir(d));
}
