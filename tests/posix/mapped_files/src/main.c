/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mapped_files_tests.h"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#ifndef CONFIG_NATIVE_LIBC
#include <ff.h>
#include <zephyr/fs/fs.h>
#endif

ZTEST_BMEM size_t mf_page;

#ifndef CONFIG_NATIVE_LIBC
static FATFS fat_fs;
static struct fs_mount_t fs_mnt = {
	.type = FS_FATFS,
	.mnt_point = "/",
	.fs_data = &fat_fs,
};
#endif

static void remove_file(void)
{
	struct stat st;

	if (stat(MF_FILE, &st) == 0) {
		zassert_ok(unlink(MF_FILE));
	}
}

static void before(void *arg)
{
	uint8_t buf[64];
	int fd;

	ARG_UNUSED(arg);

	remove_file();
	fd = open(MF_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	zassert_true(fd >= 0, "open(%s) failed: %d", MF_FILE, errno);

	for (size_t pos = 0; pos < mf_file_size(); pos += sizeof(buf)) {
		size_t n = MIN(sizeof(buf), mf_file_size() - pos);

		for (size_t i = 0; i < n; ++i) {
			buf[i] = mf_byte(pos + i);
		}
		zassert_equal(write(fd, buf, n), (ssize_t)n);
	}

	zassert_ok(close(fd));
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	remove_file();
}

static void *setup(void)
{
	long page = sysconf(_SC_PAGESIZE);

	zassert_true(page > 0);
	mf_page = (size_t)page;

#ifndef CONFIG_NATIVE_LIBC
	memset(&fat_fs, 0, sizeof(fat_fs));
	zassert_ok(fs_mount(&fs_mnt));
#endif
	(void)mkdir(MF_TMPDIR, 0777);

	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);
#ifndef CONFIG_NATIVE_LIBC
	(void)fs_unmount(&fs_mnt);
#endif
}

ZTEST_SUITE(posix_mapped_files, NULL, setup, before, after, teardown);
