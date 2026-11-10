/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <errno.h>
#include <unistd.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>

ZTEST_USER(xsi_file_system, test_lockf)
{
	int fd = zvfs_open(TEST_FILE, ZVFS_O_RDWR, 0);

	zassert_true(fd >= 0, "errno %d", errno);

	/* from the offset to the end; the holder's own lock does not conflict */
	zassert_ok(lockf(fd, F_LOCK, 0));
	zassert_ok(lockf(fd, F_TEST, 0));
	zassert_ok(lockf(fd, F_TLOCK, 0));
	zassert_ok(lockf(fd, F_ULOCK, 0));

	/* a region before the offset */
	zassert_equal(zvfs_lseek(fd, 8, ZVFS_SEEK_SET), 8);
	zassert_ok(lockf(fd, F_TLOCK, -4));
	zassert_ok(lockf(fd, F_ULOCK, -4));
	errno = 0;
	zassert_equal(lockf(fd, F_LOCK, -16), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(lockf(fd, 42, 0), -1);
	zassert_equal(errno, EINVAL);
	zassert_ok(zvfs_close(fd));

	errno = 0;
	zassert_equal(lockf(-1, F_LOCK, 0), -1);
	zassert_equal(errno, EBADF);
}
