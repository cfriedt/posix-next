/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_glob_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
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
extern struct k_mem_partition zvfs_dir_partition;
#endif

#ifndef CONFIG_NATIVE_LIBC
static FATFS fat_fs;
static struct fs_mount_t fs_mnt = {
	.type = FS_FATFS,
	.mnt_point = "/",
	.fs_data = &fat_fs,
};
#endif

/*
 * Remove a whole subtree. The Zephyr path uses the fs_* API directly: POSIX
 * <dirent.h> and the FATFS ff.h both define a DIR type, so they cannot share
 * a translation unit. Under the host libc, plain POSIX calls are used.
 */
#ifdef CONFIG_NATIVE_LIBC
#include <dirent.h>
static void rmtree(const char *path)
{
	DIR *d = opendir(path);
	struct dirent *e;
	char child[300];

	if (d != NULL) {
		while ((e = readdir(d)) != NULL) {
			if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
				continue;
			}
			if (snprintf(child, sizeof(child), "%s/%s", path, e->d_name) >=
			    (int)sizeof(child)) {
				continue;
			}
			rmtree(child);
		}
		closedir(d);
		rmdir(path);
		return;
	}

	(void)unlink(path);
}
#else
static void rmtree(const char *path)
{
	struct fs_dir_t dir;
	struct fs_dirent ent;
	char child[300];

	fs_dir_t_init(&dir);

	if (fs_opendir(&dir, path) == 0) {
		while (fs_readdir(&dir, &ent) == 0 && ent.name[0] != '\0') {
			if (snprintf(child, sizeof(child), "%s/%s", path, ent.name) >=
			    (int)sizeof(child)) {
				continue;
			}
			rmtree(child);
		}
		fs_closedir(&dir);
	}

	(void)fs_unlink(path);
}
#endif

static void write_file(const char *path, const char *content)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);

	zassert_true(fd >= 0, "open(%s) failed: %d", path, errno);
	zassert_equal(write(fd, content, strlen(content)), strlen(content));
	zassert_ok(close(fd));
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	rmtree(TEST_ROOT);

	zassert_ok(mkdir(TEST_ROOT, 0777));

	write_file(TEST_ROOT "/a.txt", "a");
	write_file(TEST_ROOT "/b.txt", "b");
	write_file(TEST_ROOT "/c.dat", "c");
	write_file(TEST_BRACKET, "[]");
	write_file(TEST_HIDDEN, ".");

	zassert_ok(mkdir(TEST_SUB, 0777));
	write_file(TEST_SUB "/x.txt", "x");
	write_file(TEST_SUB "/y.txt", "y");
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	{ int rc = chdir("/"); ARG_UNUSED(rc); }
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
#ifndef CONFIG_NATIVE_LIBC
	rmtree(TEST_ROOT);
	(void)fs_unmount(&fs_mnt);
#endif
}

ZTEST_SUITE(posix_file_system_glob, NULL, setup, before, after, teardown);
