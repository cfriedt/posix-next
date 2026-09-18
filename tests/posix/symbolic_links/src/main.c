/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "symbolic_links_tests.h"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/kernel.h>

#ifndef CONFIG_NATIVE_LIBC
#include <zephyr/fs/fs.h>
#endif
#ifdef CONFIG_USERSPACE
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/internal/fdtable_priv.h>

extern struct k_mem_partition zvfs_dir_partition;
#endif

ZTEST_BMEM int fs_test_fd = -1;

#ifndef CONFIG_NATIVE_LIBC
static struct fs_mount_t fs_mnt = {
	.type = FS_EXT2,
	.mnt_point = "/",
	.storage_dev = (void *)"RAM",
};
#endif

static void grant_fd(int fd)
{
#ifdef CONFIG_USERSPACE
	if (fd >= 0) {
		k_object_access_all_grant(zvfs_fd_entry_get(fd));
	}
#else
	ARG_UNUSED(fd);
#endif
}

void fs_test_reset(void)
{
	int fd;

	(void)unlink(TEST_LINK);
	(void)unlink(TEST_DANGLE);
	(void)unlink(TEST_NOENT);
	(void)unlink(TEST_FILE);
	(void)rmdir(TEST_ROOT);
	zassert_ok(mkdir(TEST_ROOT, 0777));
	fd = open(TEST_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	zassert_true(fd >= 0);
	zassert_equal(write(fd, TEST_CONTENT, strlen(TEST_CONTENT)), strlen(TEST_CONTENT));
	zassert_ok(close(fd));
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	fs_test_reset();
	fs_test_fd = open(TEST_FILE, O_RDWR);
	zassert_true(fs_test_fd >= 0);
	grant_fd(fs_test_fd);
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	if (fs_test_fd >= 0) {
		close(fs_test_fd);
		fs_test_fd = -1;
	}
}

static void *setup(void)
{
#ifndef CONFIG_NATIVE_LIBC
	zassert_ok(fs_mkfs(FS_EXT2, (uintptr_t)"RAM", NULL, 0));
	zassert_ok(fs_mount(&fs_mnt));
#endif
	(void)mkdir(FS_TMPDIR, 0777);
#ifdef CONFIG_USERSPACE
	zassert_ok(k_mem_domain_add_partition(&k_mem_domain_default, &zvfs_dir_partition));
#endif

	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);

	fs_test_reset();
	(void)unlink(TEST_FILE);
	(void)rmdir(TEST_ROOT);
#ifndef CONFIG_NATIVE_LIBC
	(void)fs_unmount(&fs_mnt);
#endif
}

ZTEST_SUITE(posix_symbolic_links, NULL, setup, before, after, teardown);
