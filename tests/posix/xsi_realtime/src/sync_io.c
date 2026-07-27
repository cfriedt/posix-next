/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <zephyr/ztest.h>

static const char test_str[] = "Hello World!";

#if defined(CONFIG_NATIVE_LIBC)

#include <errno.h>
#include <sys/stat.h>

#define TEST_MNTP "/tmp/posix_xsi_realtime_test"
#define TEST_FILE TEST_MNTP "/testfile.txt"

static void test_mount(void)
{
	if (mkdir(TEST_MNTP, 0770) != 0) {
		zassert_equal(errno, EEXIST, "mkdir(%s) failed: %d", TEST_MNTP, errno);
	}
}

void test_unmount(void)
{
	(void)unlink(TEST_FILE);
	(void)rmdir(TEST_MNTP);
}

#else

#include <ff.h>
#include <zephyr/fs/fs.h>

#define FATFS_MNTP "/RAM:"
#define TEST_FILE  FATFS_MNTP "/testfile.txt"

static FATFS fat_fs;

static struct fs_mount_t fatfs_mnt = {
	.type = FS_FATFS,
	.mnt_point = FATFS_MNTP,
	.fs_data = &fat_fs,
};

static void test_mount(void)
{
	int res;

	res = fs_mount(&fatfs_mnt);
	zassert_ok(res, "Error mounting fs [%d]\n", res);
}

void test_unmount(void)
{
	int res;

	res = fs_unmount(&fatfs_mnt);
	zassert_ok(res, "Error unmounting fs [%d]", res);
}

#endif

static int file_open(void)
{
	int res;

	res = open(TEST_FILE, O_CREAT | O_RDWR, 0660);
	zassert_not_equal(res, -1, "Error opening file [%d], errno [%d]", res, errno);
	return res;
}

static int file_write(int file)
{
	ssize_t brw;
	off_t res;

	res = lseek(file, 0, SEEK_SET);
	zassert_ok((int)res, "lseek failed [%d]\n", (int)res);

	brw = write(file, (char *)test_str, strlen(test_str));
	zassert_ok((int)res, "Failed writing to file [%d]\n", (int)brw);

	zassert_ok(brw < strlen(test_str),
		   "Unable to complete write. Volume full. Number of bytes written: [%d]\n",
		   (int)brw);
	return res;
}

/**
 * @brief Test for POSIX fsync API
 *
 * @details Test sync the file through POSIX fsync API.
 */
ZTEST(xsi_realtime, test_fs_sync)
{
	test_mount();
	int res = 0;
	int file = file_open();

	res = file_write(file);
	res = fsync(file);
	zassert_ok(res, "Failed to sync file: %d, errno = %d\n", res, errno);
	zassert_ok(close(file), "Failed to close file");
	test_unmount();
}

/**
 * @brief Test for POSIX fdatasync API
 *
 * @details Test sync the file through POSIX fdatasync API.
 */
ZTEST(xsi_realtime, test_fs_datasync)
{
	test_mount();
	int res = 0;
	int file = file_open();

	res = file_write(file);
	res = fdatasync(file);
	zassert_ok(res, "Failed to sync file: %d, errno = %d\n", res, errno);
	zassert_ok(close(file), "Failed to close file");
	test_unmount();
}
