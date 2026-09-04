/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_ext_tests.h"

#include <dirent.h>

ZTEST_USER(posix_file_system_ext, test_dirfd)
{
	DIR *dirp = opendir(TEST_ROOT);
	int fd;

	zassert_not_null(dirp);

	fd = dirfd(dirp);
	zassert_true(fd >= 0);

	/* the descriptor is the stream's own: stable across reads */
	zassert_not_null(readdir(dirp));
	zassert_equal(dirfd(dirp), fd);

	zassert_ok(closedir(dirp));
}
