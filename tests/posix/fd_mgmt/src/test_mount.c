/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/fs/fs.h>
#include <ff.h>
#include <zephyr/ztest.h>

#include "test_fs.h"

#if !defined(CONFIG_NATIVE_LIBC)

static FATFS fat_fs;

static struct fs_mount_t fatfs_mnt = {
	.type = FS_FATFS,
	.mnt_point = FATFS_MNTP,
	.fs_data = &fat_fs,
};

#endif

void *test_mount(void)
{
#if !defined(CONFIG_NATIVE_LIBC)
	zassert_ok(fs_mount(&fatfs_mnt));
#endif
	return NULL;
}

void test_unmount(void *unused)
{
	ARG_UNUSED(unused);
#if !defined(CONFIG_NATIVE_LIBC)
	zassert_ok(fs_unmount(&fatfs_mnt));
#endif
}
