/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mapped_files_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static void munmap_invalid(void)
{
	uint8_t *p;

	p = mmap(NULL, mf_page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);

	errno = 0;
	zassert_equal(munmap(p, 0), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(munmap(p + 1, mf_page), -1);
	zassert_equal(errno, EINVAL);

	zassert_ok(munmap(p, mf_page));
}

static void munmap_unmapped(void)
{
	uint8_t *p;

	p = mmap(NULL, mf_page, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	zassert_ok(munmap(p, mf_page));

	/* a range with no mapping in it is not an error */
	zassert_ok(munmap(p, mf_page));
}

static void munmap_writeback(int fd)
{
	uint8_t *p;
	uint8_t b;

	p = mmap(NULL, mf_page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	p[3] = (uint8_t)~mf_byte(3);
	zassert_ok(munmap(p, mf_page));

	/* unmapping a shared mapping writes its modifications back */
	zassert_equal(pread(fd, &b, 1, 3), 1);
	zassert_equal(b, (uint8_t)~mf_byte(3));
}

static void munmap_reuse(void)
{
	uint8_t *p;

	for (int i = 0; i < 2 * CONFIG_ZVFS_MMAP_MAX; ++i) {
		p = mmap(NULL, mf_page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		zassert_not_equal(p, MAP_FAILED, "mmap() %d failed: %d", i, errno);
		p[0] = (uint8_t)i;
		zassert_ok(munmap(p, mf_page));
	}
}

static void munmap_partial(void)
{
	uint8_t *p;

	p = mmap(NULL, 2 * mf_page, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);

	/* part of a mapping cannot be unmapped on its own */
	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(munmap(p, mf_page), -1);
		zassert_equal(errno, EINVAL);
	})

	zassert_ok(munmap(p, 2 * mf_page));
}

ZTEST_USER(posix_mapped_files, test_munmap)
{
	int fd = open(MF_FILE, O_RDWR);

	zassert_true(fd >= 0, "open(%s) failed: %d", MF_FILE, errno);

	munmap_invalid();
	munmap_unmapped();
	munmap_writeback(fd);
	munmap_reuse();
	munmap_partial();

	zassert_ok(close(fd));
}
