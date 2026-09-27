/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mapped_files_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/ztest_error_hook.h>

static void expect_file_byte(int fd, size_t off, uint8_t expected)
{
	uint8_t b = ~expected;

	zassert_equal(pread(fd, &b, 1, off), 1, "pread(%zu) failed: %d", off, errno);
	zassert_equal(b, expected, "file[%zu] is 0x%02x, expected 0x%02x", off, b, expected);
}

static void expect_mapped_file(const uint8_t *p, size_t off, size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		zassert_equal(p[i], mf_byte(off + i), "mapping[%zu] is 0x%02x, expected 0x%02x", i,
			      p[i], mf_byte(off + i));
	}
}

static void mmap_invalid(int fd)
{
	int dfd;

	errno = 0;
	zassert_equal(mmap(NULL, 0, PROT_READ, MAP_PRIVATE, fd, 0), MAP_FAILED);
	zassert_equal(errno, EINVAL);

	/* Linux reads MAP_SHARED | MAP_PRIVATE as its MAP_SHARED_VALIDATE extension */
	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(mmap(NULL, mf_page, PROT_READ, MAP_PRIVATE | MAP_SHARED, fd, 0),
			      MAP_FAILED);
		zassert_equal(errno, EINVAL);
	})

	errno = 0;
	zassert_equal(mmap(NULL, mf_page, PROT_READ, 0, fd, 0), MAP_FAILED);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(mmap(NULL, mf_page, PROT_READ, MAP_PRIVATE, fd, mf_page + 1), MAP_FAILED);
	zassert_equal(errno, EINVAL);

	/* a descriptor that is not open */
	dfd = open(MF_FILE, O_RDONLY);
	zassert_true(dfd >= 0);
	zassert_ok(close(dfd));
	errno = 0;
	zassert_equal(mmap(NULL, mf_page, PROT_READ, MAP_PRIVATE, dfd, 0), MAP_FAILED);
	zassert_equal(errno, EBADF);

	/* a directory cannot be mapped */
	dfd = open(MF_TMPDIR, O_RDONLY | O_DIRECTORY);
	zassert_true(dfd >= 0, "open(%s) failed: %d", MF_TMPDIR, errno);
	errno = 0;
	zassert_equal(mmap(NULL, mf_page, PROT_READ, MAP_PRIVATE, dfd, 0), MAP_FAILED);
	zassert_equal(errno, ENODEV);
	zassert_ok(close(dfd));

	/* a shared writable mapping needs a descriptor open for writing */
	dfd = open(MF_FILE, O_RDONLY);
	zassert_true(dfd >= 0);
	errno = 0;
	zassert_equal(mmap(NULL, mf_page, PROT_READ | PROT_WRITE, MAP_SHARED, dfd, 0), MAP_FAILED);
	zassert_equal(errno, EACCES);
	zassert_ok(close(dfd));

	/* fixed placement is not supported */
	IF_NOT_NATIVE_LIBC({
		void *hint = (void *)(uintptr_t)(16 * mf_page);

		int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED;

		errno = 0;
		zassert_equal(mmap(hint, mf_page, PROT_READ, flags, -1, 0), MAP_FAILED);
		zassert_equal(errno, ENOTSUP);
	})
}

static void mmap_anonymous(void)
{
	uint8_t *p;
	uint8_t *q;

	p = mmap(NULL, 2 * mf_page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	zassert_true(((uintptr_t)p % mf_page) == 0, "%p is not page aligned", p);

	for (size_t i = 0; i < 2 * mf_page; ++i) {
		zassert_equal(p[i], 0, "anonymous page not zero-filled at %zu", i);
	}
	for (size_t i = 0; i < 2 * mf_page; ++i) {
		p[i] = mf_byte(i);
	}
	for (size_t i = 0; i < 2 * mf_page; ++i) {
		zassert_equal(p[i], mf_byte(i));
	}

	q = mmap(NULL, mf_page, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(q, MAP_FAILED, "mmap() failed: %d", errno);
	zassert_equal(q[mf_page - 1], 0);
	q[mf_page - 1] = 0x5a;
	zassert_equal(q[mf_page - 1], 0x5a);

	zassert_ok(munmap(q, mf_page));
	zassert_ok(munmap(p, 2 * mf_page));
}

static void mmap_private(int fd)
{
	size_t len = mf_file_pages_rounded();
	uint8_t *p;

	p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	expect_mapped_file(p, 0, mf_file_size());

	/* modifications stay private, even when synchronized */
	p[0] = (uint8_t)~mf_byte(0);
	p[mf_page] = (uint8_t)~mf_byte(mf_page);
	zassert_ok(msync(p, len, MS_SYNC));
	expect_file_byte(fd, 0, mf_byte(0));
	expect_file_byte(fd, mf_page, mf_byte(mf_page));

	zassert_ok(munmap(p, len));
	expect_file_byte(fd, 0, mf_byte(0));
}

static void mmap_shared(int fd)
{
	size_t len = mf_file_pages_rounded();
	size_t last = mf_file_size() - 1;
	uint8_t *p;

	p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	expect_mapped_file(p, 0, mf_file_size());

	/* modifications reach the file when synchronized */
	p[0] = (uint8_t)~mf_byte(0);
	p[2 * mf_page + 5] = (uint8_t)~mf_byte(2 * mf_page + 5);
	p[last] = (uint8_t)~mf_byte(last);
	zassert_ok(msync(p, len, MS_SYNC));
	expect_file_byte(fd, 0, (uint8_t)~mf_byte(0));
	expect_file_byte(fd, 2 * mf_page + 5, (uint8_t)~mf_byte(2 * mf_page + 5));
	expect_file_byte(fd, last, (uint8_t)~mf_byte(last));
	expect_file_byte(fd, 1, mf_byte(1));

	/* the file does not grow past its size */
	{
		struct stat st;

		zassert_ok(fstat(fd, &st));
		zassert_equal((size_t)st.st_size, mf_file_size());
	}

	p[0] = mf_byte(0);
	p[2 * mf_page + 5] = mf_byte(2 * mf_page + 5);
	p[last] = mf_byte(last);
	zassert_ok(msync(p, len, MS_SYNC));
	zassert_ok(munmap(p, len));
	expect_file_byte(fd, 0, mf_byte(0));
}

static void mmap_offset(int fd)
{
	uint8_t *p;

	p = mmap(NULL, mf_page, PROT_READ, MAP_PRIVATE, fd, mf_page);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	expect_mapped_file(p, mf_page, mf_page);
	zassert_ok(munmap(p, mf_page));

	p = mmap(NULL, mf_page, PROT_READ, MAP_SHARED, fd, MF_FILE_PAGES * mf_page);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	expect_mapped_file(p, MF_FILE_PAGES * mf_page, mf_page / 2);
	zassert_ok(munmap(p, mf_page));
}

static void mmap_past_eof(int fd)
{
	size_t len = mf_file_pages_rounded();
	uint8_t *p;

	/* the part of the last page beyond the end of the file reads as zero */
	p = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	zassert_equal(p[mf_file_size() - 1], mf_byte(mf_file_size() - 1));
	for (size_t i = mf_file_size(); i < len; ++i) {
		zassert_equal(p[i], 0, "byte %zu past EOF is 0x%02x", i, p[i]);
	}
	zassert_ok(munmap(p, len));
}

static void mmap_outlives_descriptor(void)
{
	uint8_t *p;
	uint8_t b;
	int fd;

	fd = open(MF_FILE, O_RDWR);
	zassert_true(fd >= 0);
	p = mmap(NULL, mf_page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	zassert_ok(close(fd));

	/* the mapping stays valid and synchronizable after the descriptor is closed */
	expect_mapped_file(p, 0, mf_page);
	p[1] = (uint8_t)~mf_byte(1);
	zassert_ok(msync(p, mf_page, MS_SYNC));
	zassert_ok(munmap(p, mf_page));

	fd = open(MF_FILE, O_RDWR);
	zassert_true(fd >= 0);
	expect_file_byte(fd, 1, (uint8_t)~mf_byte(1));
	expect_file_byte(fd, 0, mf_byte(0));
	b = mf_byte(1);
	zassert_equal(pwrite(fd, &b, 1, 1), 1);
	zassert_ok(close(fd));
}

static void mmap_several(int fd)
{
	uint8_t *p[3];

	p[0] = mmap(NULL, mf_page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	p[1] = mmap(NULL, mf_page, PROT_READ, MAP_PRIVATE, fd, 0);
	p[2] = mmap(NULL, mf_page, PROT_READ, MAP_SHARED, fd, mf_page);
	for (size_t i = 0; i < ARRAY_SIZE(p); ++i) {
		zassert_not_equal(p[i], MAP_FAILED, "mmap() %zu failed: %d", i, errno);
		for (size_t j = 0; j < i; ++j) {
			zassert_true((p[i] + mf_page <= p[j]) || (p[j] + mf_page <= p[i]),
				     "mappings %zu and %zu overlap", i, j);
		}
	}

	p[0][0] = 0xa5;
	expect_mapped_file(p[1], 0, mf_page);
	expect_mapped_file(p[2], mf_page, mf_page);

	for (size_t i = 0; i < ARRAY_SIZE(p); ++i) {
		zassert_ok(munmap(p[i], mf_page));
	}
}

ZTEST_USER(posix_mapped_files, test_mmap)
{
	int fd = open(MF_FILE, O_RDWR);

	zassert_true(fd >= 0, "open(%s) failed: %d", MF_FILE, errno);

	mmap_invalid(fd);
	mmap_anonymous();
	mmap_private(fd);
	mmap_shared(fd);
	mmap_offset(fd);
	mmap_past_eof(fd);
	mmap_outlives_descriptor();
	mmap_several(fd);

	zassert_ok(close(fd));
}

ZTEST_USER(posix_mapped_files, test_mmap_fault)
{
	volatile uint8_t *p;

	/* an MMU protects the kernel's own view too; an MPU only protects user threads */
	if (IS_ENABLED(CONFIG_NATIVE_LIBC) ||
	    (!IS_ENABLED(CONFIG_MMU) && !IS_ENABLED(CONFIG_USERSPACE))) {
		ztest_test_skip();
	}

	p = mmap(NULL, mf_page, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	zassert_equal(p[0], 0, "p[0] is 0x%02x", p[0]);

	ztest_set_fault_valid(true);
	p[0] = 1;
	ztest_test_fail();
}
