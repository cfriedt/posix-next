/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#include "test_fs.h"

#define RENAME_OLD TEST_MNTP "/rename1.txt"
#define RENAME_NEW TEST_MNTP "/rename2.txt"

ZTEST(posix_fs_test, test_rename)
{
	int fd;

	fd = open(RENAME_OLD, O_CREAT | O_RDWR, 0660);
	zassert_true(fd >= 0, "open() failed, errno=%d", errno);
	zassert_ok(close(fd));

	zassert_ok(rename(RENAME_OLD, RENAME_NEW), "rename() failed, errno=%d", errno);

	/* the old name must be gone */
	zassert_equal(unlink(RENAME_OLD), -1);
	zassert_ok(unlink(RENAME_NEW));
}
