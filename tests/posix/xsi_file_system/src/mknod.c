/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <errno.h>
#include <sys/stat.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs_fs.h>

ZTEST_USER(xsi_file_system, test_mknod)
{
	struct zvfs_stat st;

	zassert_ok(mknod(TEST_SCRATCH, S_IFIFO | 0644, 0));
	zassert_ok(zvfs_stat(TEST_SCRATCH, &st));
	zassert_equal(st.mode & ZVFS_MODE_IFMT, ZVFS_MODE_IFIFO);
	errno = 0;
	zassert_equal(mknod(TEST_SCRATCH, S_IFIFO | 0644, 0), -1);
	zassert_equal(errno, EEXIST);
	zassert_ok(zvfs_unlink(TEST_SCRATCH));

	/* a regular file, empty; no type at all is a regular file too */
	zassert_ok(mknod(TEST_SCRATCH, S_IFREG | 0644, 0));
	zassert_ok(zvfs_stat(TEST_SCRATCH, &st));
	zassert_equal(st.mode & ZVFS_MODE_IFMT, ZVFS_MODE_IFREG);
	zassert_equal(st.size, 0);
	zassert_ok(zvfs_unlink(TEST_SCRATCH));
	zassert_ok(mknod(TEST_SCRATCH, 0644, 0));
	zassert_ok(zvfs_stat(TEST_SCRATCH, &st));
	zassert_equal(st.mode & ZVFS_MODE_IFMT, ZVFS_MODE_IFREG);
	zassert_ok(zvfs_unlink(TEST_SCRATCH));

	/* no device special files exist to make */
	errno = 0;
	zassert_equal(mknod(TEST_SCRATCH, S_IFCHR | 0644, 0), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(mknod(TEST_NOENT "/x", S_IFREG | 0644, 0), -1);
	zassert_equal(errno, ENOENT);
}
