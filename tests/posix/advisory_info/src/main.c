/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "advisory_info_tests.h"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#ifndef CONFIG_NATIVE_LIBC
#include <ff.h>
#include <zephyr/fs/fs.h>
#endif
#ifdef CONFIG_USERSPACE
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/internal/fdtable_priv.h>
extern struct k_mem_partition zvfs_dir_partition;
#endif

ZTEST_BMEM int fs_test_fd = -1;
ZTEST_BMEM int fs_test_rofd = -1;
ZTEST_BMEM int fs_test_dirfd = -1;

#ifndef CONFIG_NATIVE_LIBC
static FATFS fat_fs;
static struct fs_mount_t fs_mnt = {
	.type = FS_FATFS,
	.mnt_point = "/",
	.fs_data = &fat_fs,
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

static int open_granted(const char *path, int flags)
{
	int fd = open(path, flags);

	zassert_true(fd >= 0, "open(%s) failed: %d", path, errno);
	grant_fd(fd);
	return fd;
}

static void before(void *arg)
{
	int fd;

	ARG_UNUSED(arg);

	(void)unlink(TEST_FILE);
	(void)rmdir(TEST_ROOT);
	zassert_ok(mkdir(TEST_ROOT, 0777));
	fd = open(TEST_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	zassert_true(fd >= 0);
	zassert_equal(write(fd, TEST_CONTENT, strlen(TEST_CONTENT)), strlen(TEST_CONTENT));
	zassert_ok(close(fd));

	fs_test_fd = open_granted(TEST_FILE, O_RDWR);
	fs_test_rofd = open_granted(TEST_FILE, O_RDONLY);
	fs_test_dirfd = open_granted(TEST_ROOT, O_RDONLY | O_DIRECTORY);
}

static void after(void *arg)
{
	int *fds[] = {&fs_test_fd, &fs_test_rofd, &fs_test_dirfd};

	ARG_UNUSED(arg);

	ARRAY_FOR_EACH_PTR(fds, fd) {
		if (**fd >= 0) {
			close(**fd);
			**fd = -1;
		}
	}
}

static void *setup(void)
{
#ifndef CONFIG_NATIVE_LIBC
	memset(&fat_fs, 0, sizeof(fat_fs));
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

	(void)unlink(TEST_FILE);
	(void)rmdir(TEST_ROOT);
#ifndef CONFIG_NATIVE_LIBC
	(void)fs_unmount(&fs_mnt);
#endif
}

ZTEST_SUITE(posix_advisory_info, NULL, setup, before, after, teardown);
