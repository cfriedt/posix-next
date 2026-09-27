/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <ff.h>
#include <sys/mman.h>
#include <unistd.h>

#include <zephyr/fs/fs.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

#define FATFS_MNTP "/RAM:"
/* the host libc opens host paths: no volume to mount, a temporary file instead */
#define MSYNC_FILE                                                                                 \
	COND_CODE_1(CONFIG_NATIVE_LIBC, ("/tmp/posix_xsi_realtime_msync.bin"),                     \
		    (FATFS_MNTP "/msync.bin"))

/* the file holds this many pages */
#define MSYNC_FILE_PAGES 2

static FATFS msync_fat_fs;

static struct fs_mount_t msync_mnt = {
	.type = FS_FATFS,
	.mnt_point = FATFS_MNTP,
	.fs_data = &msync_fat_fs,
};

static size_t page;

/* the byte the fixture writes at file offset i */
static inline uint8_t file_byte(size_t i)
{
	return (uint8_t)(i * 7 + (i >> 8));
}

static int msync_setup(void)
{
	uint8_t buf[64];
	long ps;
	int fd;

	ps = sysconf(_SC_PAGESIZE);
	zassert_true(ps > 0);
	page = (size_t)ps;

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		zassert_ok(fs_mount(&msync_mnt));
	}

	fd = open(MSYNC_FILE, O_RDWR | O_CREAT | O_TRUNC, 0600);
	zassert_true(fd >= 0, "open(%s) failed: %d", MSYNC_FILE, errno);
	for (size_t pos = 0; pos < MSYNC_FILE_PAGES * page; pos += sizeof(buf)) {
		for (size_t i = 0; i < sizeof(buf); ++i) {
			buf[i] = file_byte(pos + i);
		}
		zassert_equal(write(fd, buf, sizeof(buf)), (ssize_t)sizeof(buf));
	}

	return fd;
}

static void msync_teardown(int fd)
{
	zassert_ok(close(fd));
	zassert_ok(unlink(MSYNC_FILE));
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		zassert_ok(fs_unmount(&msync_mnt));
	}
}

static void msync_invalid(void)
{
	uint8_t *p;

	p = mmap(NULL, page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);

	errno = 0;
	zassert_equal(msync(p, page, 0x80), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(msync(p, page, MS_SYNC | MS_ASYNC), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(msync(p + 1, page, MS_SYNC), -1);
	zassert_equal(errno, EINVAL);

	/* an empty range is fine, an unmapped one is not */
	zassert_ok(msync(p, 0, MS_SYNC));
	zassert_ok(munmap(p, page));
	errno = 0;
	zassert_equal(msync(p, page, MS_SYNC), -1);
	zassert_equal(errno, ENOMEM);
}

static void msync_writeback(int fd, int flags)
{
	uint8_t *p;
	uint8_t b;

	p = mmap(NULL, 2 * page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);

	p[page + 7] = (uint8_t)~file_byte(page + 7);
	zassert_ok(msync(p + page, page, flags));
	zassert_equal(pread(fd, &b, 1, page + 7), 1);
	zassert_equal(b, (uint8_t)~file_byte(page + 7));

	p[page + 7] = file_byte(page + 7);
	zassert_ok(msync(p, 2 * page, flags));
	zassert_equal(pread(fd, &b, 1, page + 7), 1);
	zassert_equal(b, file_byte(page + 7));

	zassert_ok(munmap(p, 2 * page));
}

static void msync_invalidate(int fd)
{
	uint8_t *p;
	uint8_t b = (uint8_t)~file_byte(9);

	p = mmap(NULL, page, PROT_READ, MAP_SHARED, fd, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	zassert_equal(p[9], file_byte(9));

	/* a change written through the descriptor shows once the pages are invalidated */
	zassert_equal(pwrite(fd, &b, 1, 9), 1);
	zassert_ok(msync(p, page, MS_INVALIDATE));
	zassert_equal(p[9], b);

	zassert_ok(msync(p, page, MS_SYNC | MS_INVALIDATE));
	zassert_equal(p[9], b);
	zassert_equal(p[8], file_byte(8));

	zassert_ok(munmap(p, page));
}

/* only the pages modified through the mapping reach the file */
static void msync_dirty_pages_only(int fd)
{
	uint8_t *p;
	uint8_t b;
	uint8_t via_fd = (uint8_t)~file_byte(page + 3);

	p = mmap(NULL, 2 * page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);

	/* the descriptor changes page 1 while the mapping changes page 0 */
	zassert_equal(pwrite(fd, &via_fd, 1, page + 3), 1);
	p[5] = (uint8_t)~file_byte(5);
	zassert_ok(msync(p, 2 * page, MS_SYNC));

	zassert_equal(pread(fd, &b, 1, 5), 1);
	zassert_equal(b, (uint8_t)~file_byte(5));
	zassert_equal(pread(fd, &b, 1, page + 3), 1);
	zassert_equal(b, via_fd, "an unmodified page was written back over the descriptor's write");

	/* a page that has been written back is tracked again */
	p[5] = file_byte(5);
	p[page + 9] = (uint8_t)~file_byte(page + 9);
	zassert_ok(msync(p, 2 * page, MS_SYNC));
	zassert_equal(pread(fd, &b, 1, 5), 1);
	zassert_equal(b, file_byte(5));
	zassert_equal(pread(fd, &b, 1, page + 9), 1);
	zassert_equal(b, (uint8_t)~file_byte(page + 9));

	zassert_ok(munmap(p, 2 * page));
}

ZTEST(xsi_realtime, test_msync)
{
	int fd = msync_setup();

	msync_invalid();
	msync_writeback(fd, MS_SYNC);
	msync_writeback(fd, MS_ASYNC);
	msync_invalidate(fd);
	if (IS_ENABLED(CONFIG_ZVFS_MMAP_DIRTY_TRACK) || IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		msync_dirty_pages_only(fd);
	}

	msync_teardown(fd);
}
