/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_tests.h"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

static mode_t perm_of(const char *path)
{
	struct stat st;

	zassert_ok(stat(path, &st), "stat %s: %d", path, errno);

	return st.st_mode & 07777;
}

ZTEST_USER(posix_file_attributes, test_chmod)
{
	/* the permission bits are stored and reported; the type is the file's */
	zassert_ok(chmod(TEST_FILE, 0644));
	zassert_equal(perm_of(TEST_FILE), 0644, "0%o", perm_of(TEST_FILE));
	zassert_ok(chmod(TEST_FILE, S_IFDIR | 0600));
	zassert_equal(perm_of(TEST_FILE), 0600, "0%o", perm_of(TEST_FILE));
	zassert_ok(chmod(TEST_DIR, 0755));
	zassert_equal(perm_of(TEST_DIR), 0755, "0%o", perm_of(TEST_DIR));

	/* execute permission is what access(X_OK) checks, for the privileged identity too */
	errno = 0;
	zassert_equal(access(TEST_FILE, X_OK), -1);
	zassert_equal(errno, EACCES);
	zassert_ok(chmod(TEST_FILE, 0700));
	zassert_ok(access(TEST_FILE, X_OK));

	errno = 0;
	zassert_equal(chmod(TEST_NOENT, 0644), -1);
	zassert_equal(errno, ENOENT);
}
