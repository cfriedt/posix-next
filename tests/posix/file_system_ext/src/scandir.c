/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_ext_tests.h"

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static int select_f(const struct dirent *entry)
{
	return entry->d_name[0] == 'f';
}

ZTEST_USER(posix_file_system_ext, test_scandir)
{
	static const char *const expected[] = {"f1.txt", "f2.txt", "f3.txt"};
	struct dirent **namelist = NULL;
	int count;

	count = scandir(TEST_SCAN, &namelist, select_f, alphasort);
	zassert_equal(count, ARRAY_SIZE(expected));
	for (int i = 0; i < count; i++) {
		zassert_equal(strcmp(namelist[i]->d_name, expected[i]), 0, "entry %d is %s", i,
			      namelist[i]->d_name);
		free(namelist[i]);
	}
	free(namelist);

	/* unfiltered and unsorted: at least the four fixture entries */
	namelist = NULL;
	count = scandir(TEST_SCAN, &namelist, NULL, NULL);
	zassert_true(count >= 4);
	for (int i = 0; i < count; i++) {
		free(namelist[i]);
	}
	free(namelist);

	errno = 0;
	zassert_equal(scandir(TEST_ROOT "/nope", &namelist, NULL, NULL), -1);
	zassert_equal(errno, ENOENT);
}
