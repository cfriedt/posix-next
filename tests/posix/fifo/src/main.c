/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fifo_tests.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/kernel.h>

#ifndef CONFIG_NATIVE_LIBC
#include <zephyr/fs/fs.h>

static struct fs_mount_t fs_mnt = {
	.type = FS_EXT2,
	.mnt_point = "/",
	.storage_dev = (void *)"RAM",
};
#endif

static void before(void *arg)
{
	int fd;

	ARG_UNUSED(arg);

	(void)unlink(TEST_FIFO);
	(void)unlink(TEST_FILE);
	(void)rmdir(TEST_ROOT);
	zassert_ok(mkdir(TEST_ROOT, 0777));
	fd = open(TEST_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	zassert_true(fd >= 0);
	zassert_ok(close(fd));
}

static void *setup(void)
{
#ifndef CONFIG_NATIVE_LIBC
	zassert_ok(fs_mkfs(FS_EXT2, (uintptr_t)"RAM", NULL, 0));
	zassert_ok(fs_mount(&fs_mnt));
#endif
	(void)mkdir(FS_TMPDIR, 0777);

	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);

	(void)unlink(TEST_FIFO);
	(void)unlink(TEST_FILE);
	(void)rmdir(TEST_ROOT);
#ifndef CONFIG_NATIVE_LIBC
	(void)fs_unmount(&fs_mnt);
#endif
}

ZTEST_SUITE(posix_fifo, NULL, setup, before, NULL, teardown);
