/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "wide_char_device_io_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <wchar.h>
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

#ifndef CONFIG_NATIVE_LIBC
static FATFS fat_fs;
static struct fs_mount_t fs_mnt = {
	.type = FS_FATFS,
	.mnt_point = "/",
	.fs_data = &fat_fs,
};
#endif

static ZTEST_BMEM int saved_in = -1;
static ZTEST_BMEM int saved_out = -1;
static ZTEST_BMEM wchar_t out_buf[64];

static void write_wide_file(const char *path, const wchar_t *content)
{
	FILE *f = fopen(path, "w");

	zassert_not_null(f, "fopen(%s) failed: %d", path, errno);
	zassert_true(fputws(content, f) >= 0);
	zassert_ok(fclose(f));
}

FILE *test_open_in(void)
{
	FILE *f = fopen(TEST_IN, "r");

	zassert_not_null(f, "fopen(" TEST_IN ") failed: %d", errno);
	return f;
}

FILE *test_open_out(void)
{
	FILE *f = fopen(TEST_OUT, "w");

	zassert_not_null(f, "fopen(" TEST_OUT ") failed: %d", errno);
	return f;
}

const wchar_t *test_read_out(void)
{
	FILE *f = fopen(TEST_OUT, "r");
	size_t n = 0;
	wint_t wc;

	zassert_not_null(f, "fopen(" TEST_OUT ") failed: %d", errno);
	while ((n < ARRAY_SIZE(out_buf) - 1) && ((wc = fgetwc(f)) != WEOF)) {
		out_buf[n++] = (wchar_t)wc;
	}
	out_buf[n] = L'\0';
	zassert_ok(fclose(f));

	return out_buf;
}

#ifdef CONFIG_USERSPACE
static void grant_fd(int fd)
{
	k_object_access_all_grant(zvfs_fd_entry_get(fd));
}
#endif

void test_swap_std_in(void)
{
	int fd = open(TEST_IN, O_RDONLY);

	zassert_true(fd >= 0, "open(" TEST_IN ") failed: %d", errno);
	saved_in = dup(STDIN_FILENO);
	zassert_true(saved_in >= 0);
	zassert_equal(dup2(fd, STDIN_FILENO), STDIN_FILENO);
	zassert_ok(close(fd));
}

void test_swap_std_out(void)
{
	int fd = open(TEST_OUT, O_WRONLY | O_TRUNC);

	zassert_true(fd >= 0, "open(" TEST_OUT ") failed: %d", errno);
	zassert_ok(fflush(stdout));
	saved_out = dup(STDOUT_FILENO);
	zassert_true(saved_out >= 0);
	zassert_equal(dup2(fd, STDOUT_FILENO), STDOUT_FILENO);
	zassert_ok(close(fd));
}

void test_restore_std(void)
{
	if (saved_out >= 0) {
		(void)fflush(stdout);
		(void)dup2(saved_out, STDOUT_FILENO);
		(void)close(saved_out);
		saved_out = -1;
	}
	if (saved_in >= 0) {
		(void)dup2(saved_in, STDIN_FILENO);
		(void)close(saved_in);
		saved_in = -1;
	}
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	/* ztest zeroes the test partition between user-mode tests */
	saved_in = -1;
	saved_out = -1;

	(void)mkdir(TEST_ROOT, 0777);
	write_wide_file(TEST_IN, TEST_CONTENT);
	write_wide_file(TEST_OUT, L"");
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	test_restore_std();
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
	grant_fd(STDIN_FILENO);
	grant_fd(STDOUT_FILENO);
#endif
	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);

	(void)unlink(TEST_IN);
	(void)unlink(TEST_OUT);
	(void)rmdir(TEST_ROOT);
#ifndef CONFIG_NATIVE_LIBC
	(void)fs_unmount(&fs_mnt);
#endif
}

ZTEST_SUITE(posix_wide_char_device_io, NULL, setup, before, after, teardown);
