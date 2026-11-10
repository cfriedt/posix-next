/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <stdio.h>
#include <string.h>

#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>

#ifdef CONFIG_USERSPACE
extern struct k_mem_partition zvfs_dir_partition;
#endif

static struct fs_mount_t fs_mnt = {
	.type = FS_TMPFS,
	.mnt_point = "/",
};

static void tree_file(const char *path, const char *content)
{
	struct fs_file_t file;

	fs_file_t_init(&file);
	zassert_ok(fs_open(&file, path, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC));
	zassert_equal(fs_write(&file, content, strlen(content)), (ssize_t)strlen(content));
	zassert_ok(fs_close(&file));
}

/* removing an entry moves the ones behind it: take the first entry afresh each time */
static void tree_rmtree(const char *path)
{
	struct fs_dir_t dir;
	struct fs_dirent ent;
	char child[64];

	for (;;) {
		fs_dir_t_init(&dir);
		if (fs_opendir(&dir, path) != 0) {
			break;
		}
		ent.name[0] = '\0';
		(void)fs_readdir(&dir, &ent);
		(void)fs_closedir(&dir);
		if (ent.name[0] == '\0') {
			break;
		}
		if (snprintf(child, sizeof(child), "%s/%s", path, ent.name) >= (int)sizeof(child)) {
			break;
		}
		if (ent.type == FS_DIR_ENTRY_DIR) {
			tree_rmtree(child);
		} else {
			(void)fs_unlink(child);
		}
	}
	(void)fs_unlink(path);
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	tree_rmtree(TEST_ROOT);
	zassert_ok(fs_mkdir(TEST_ROOT));
	zassert_ok(fs_mkdir(TEST_DIR));
	zassert_ok(fs_mkdir(TEST_DEEP));
	zassert_ok(fs_mkdir(TEST_EMPTY));
	tree_file(TEST_FILE, TEST_CONTENT);
	tree_file(TEST_SUB, TEST_CONTENT);
	tree_file(TEST_LEAF, TEST_CONTENT);
	zassert_ok(fs_symlink("dir/sub.txt", TEST_LINK));
	zassert_ok(fs_symlink("nope", TEST_DANGLING));
}

static void *setup(void)
{
	zassert_ok(fs_mount(&fs_mnt));
	zassert_ok(fs_mkdir(FS_TMPDIR));
#ifdef CONFIG_USERSPACE
	zassert_ok(k_mem_domain_add_partition(&k_mem_domain_default, &zvfs_dir_partition));
#endif
	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);
	tree_rmtree(TEST_ROOT);
	(void)fs_unlink(FS_TMPDIR);
	(void)fs_unmount(&fs_mnt);
}

ZTEST_SUITE(xsi_file_system, NULL, setup, before, NULL, teardown);
