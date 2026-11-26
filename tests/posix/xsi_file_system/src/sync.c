/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>

ZTEST_USER(xsi_file_system, test_sync)
{
	struct zvfs_stat st;
	int fd = zvfs_open(TEST_SCRATCH, ZVFS_O_RDWR | ZVFS_O_CREAT | ZVFS_O_EXCL, 0600);

	zassert_true(fd >= 0, "errno %d", errno);
	zassert_equal(zvfs_write(fd, TEST_CONTENT, strlen(TEST_CONTENT)), strlen(TEST_CONTENT));

	/* the buffered write reaches the file; errno is left alone */
	errno = 0;
	sync();
	zassert_equal(errno, 0);
	zassert_ok(zvfs_stat(TEST_SCRATCH, &st));
	zassert_equal(st.size, (off_t)strlen(TEST_CONTENT));

	zassert_ok(zvfs_close(fd));
	zassert_ok(zvfs_unlink(TEST_SCRATCH));

	/* nothing open: nothing to do */
	sync();
}
