/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_func_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
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

static const uint8_t toybox_image[] __aligned(8) = {
#include "toybox.inc"
};

/* one fs block at a time: a single large fs_write() to ext2 corrupts the file's first blocks */
static void install(const char *path, const uint8_t *data, size_t len)
{
	struct fs_file_t f;

	fs_file_t_init(&f);
	zassert_ok(fs_open(&f, path, FS_O_CREATE | FS_O_WRITE));
	for (size_t off = 0; off < len; off += 1024) {
		zassert_true(fs_write(&f, data + off, MIN(1024, len - off)) > 0);
	}
	zassert_ok(fs_close(&f));
}
#endif

static char out_buf[128];

const char *test_read_out(void)
{
	int fd = open(TEST_OUT, O_RDONLY);
	ssize_t len;

	zassert_true(fd >= 0, "open(" TEST_OUT ") failed: %d", errno);
	len = read(fd, out_buf, sizeof(out_buf) - 1);
	zassert_true(len >= 0);
	out_buf[len] = '\0';
	zassert_ok(close(fd));

	return out_buf;
}

static void before(void *arg)
{
	int fd;

	ARG_UNUSED(arg);

	(void)unlink(TEST_OUT);
	(void)unlink(TEST_GLOB "/a1");
	(void)unlink(TEST_GLOB "/a2");
	(void)unlink(TEST_GLOB "/b1");
	(void)rmdir(TEST_GLOB);
	(void)rmdir(TEST_ROOT);
	zassert_ok(mkdir(TEST_ROOT, 0777));
	zassert_ok(mkdir(TEST_GLOB, 0777));
	static const char *const names[] = {TEST_GLOB "/a1", TEST_GLOB "/a2", TEST_GLOB "/b1"};

	ARRAY_FOR_EACH(names, i) {
		fd = open(names[i], O_WRONLY | O_CREAT, 0600);
		zassert_true(fd >= 0);
		zassert_ok(close(fd));
	}
}

static void *setup(void)
{
#ifndef CONFIG_NATIVE_LIBC
	zassert_ok(fs_mkfs(FS_EXT2, (uintptr_t)"RAM", NULL, 0));
	zassert_ok(fs_mount(&fs_mnt));
	/* the usual install: the shell is a /bin symlink to the binary, which dispatches on
	 * argv[0]
	 */
	zassert_ok(fs_mkdir("/bin"));
	install("/bin/toybox", toybox_image, sizeof(toybox_image));
	zassert_ok(fs_symlink("toybox", "/bin/sh"));
#endif
	(void)mkdir(FS_TMPDIR, 0777);
	/* what an application that spawns shells provides: where the commands are */
	zassert_ok(setenv("PATH", "/bin", 0));

	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);

	(void)unlink(TEST_OUT);
	(void)unlink(TEST_GLOB "/a1");
	(void)unlink(TEST_GLOB "/a2");
	(void)unlink(TEST_GLOB "/b1");
	(void)rmdir(TEST_GLOB);
	(void)rmdir(TEST_ROOT);
#ifndef CONFIG_NATIVE_LIBC
	(void)fs_unmount(&fs_mnt);
#endif
}

ZTEST_SUITE(posix_shell_func, NULL, setup, before, NULL, teardown);
