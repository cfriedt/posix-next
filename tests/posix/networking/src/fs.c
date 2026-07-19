/*
 * Copyright (c) 2024 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <netdb.h>

#include <ff.h>
#include <zephyr/fs/fs.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

static FATFS _fs;

static struct fs_mount_t _mnt = {
	.type = FS_FATFS,
	.mnt_point = "/",
	.fs_data = &_fs,
};

struct _entry {
	const char *const name;
	const char *const data;
};

static const struct _entry _data[] = {
	{.name = "/etc/hosts", .data = "127.0.0.1 localhost\n"},
	{.name = "/etc/protocols", .data = "tcp 6 TCP\nudp 17 UDP\n"},
	{.name = "/etc/services", .data = "http 80/tcp\n"},
	{.name = "/etc/networks", .data = "link 0\n"},
};

void *test_fs_setup(void)
{
	int ret;

	memset(&_fs, 0, sizeof(_fs));

	ret = fs_mount(&_mnt);
	zassert_ok(ret, "mount failed: %d", ret);

	ret = fs_mkdir("/etc");
	zassert_ok(ret, "mkdir failed: %d", ret);

	ARRAY_FOR_EACH_PTR(_data, entry) {
		int len;
		struct fs_file_t zfp;

		fs_file_t_init(&zfp);

		ret = fs_open(&zfp, entry->name, FS_O_CREATE | FS_O_RDWR | FS_O_TRUNC);
		zassert_true(ret >= 0, "open %s failed: %d", entry->name, ret);

		len = strlen(entry->data);
		ret = fs_write(&zfp, entry->data, len);
		zassert_equal(ret, len, "write %s returned %d", entry->name, ret);

		ret = fs_close(&zfp);
		zassert_ok(ret, "close %s returned %d", entry->name, ret);
	}

	return &_mnt;
}

void test_fs_teardown(void *arg)
{
	struct fs_mount_t *const mnt = arg;

	(void)fs_unmount(mnt);
}

void test_netdb_cleanup(void *arg)
{
	ARG_UNUSED(arg);

	endhostent();
	endnetent();
	endprotoent();
	endservent();
}
