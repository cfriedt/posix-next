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

#define REMOVE_FILE TEST_MNTP "/remove.txt"

ZTEST(posix_fs_test, test_remove)
{
	int fd;

	fd = open(REMOVE_FILE, O_CREAT | O_RDWR, 0660);
	zassert_true(fd >= 0, "open() failed, errno=%d", errno);
	zassert_ok(close(fd));

	zassert_ok(remove(REMOVE_FILE), "remove() failed, errno=%d", errno);
	zassert_equal(remove(REMOVE_FILE), -1);
}
