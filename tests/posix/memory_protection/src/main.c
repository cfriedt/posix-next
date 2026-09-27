/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#include <zephyr/ztest.h>
#include <zephyr/ztest_error_hook.h>

#include "../../shared/linux_compat_test.h"

static ZTEST_BMEM size_t page;

static void mprotect_invalid(void)
{
	uint8_t *p;

	p = mmap(NULL, page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);

	errno = 0;
	zassert_equal(mprotect(p + 1, page, PROT_READ), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(mprotect(p, page, 0x80), -1);
	zassert_equal(errno, EINVAL);

	zassert_ok(munmap(p, page));

	errno = 0;
	zassert_equal(mprotect(p, page, PROT_READ), -1);
	zassert_equal(errno, ENOMEM);
}

static void mprotect_transitions(void)
{
	uint8_t *p;

	p = mmap(NULL, page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	p[0] = 0x11;

	zassert_ok(mprotect(p, page, PROT_READ));
	zassert_equal(p[0], 0x11);

	zassert_ok(mprotect(p, page, PROT_NONE));
	zassert_ok(mprotect(p, page, PROT_READ | PROT_WRITE));
	zassert_equal(p[0], 0x11);
	p[0] = 0x22;
	zassert_equal(p[0], 0x22);

	zassert_ok(munmap(p, page));
}

static void mprotect_partial(void)
{
	uint8_t *p;

	p = mmap(NULL, 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);

	/* part of a mapping cannot change protection on its own */
	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(mprotect(p, page, PROT_READ), -1);
		zassert_equal(errno, ENOTSUP);
	})

	zassert_ok(mprotect(p, 2 * page, PROT_READ));
	zassert_ok(munmap(p, 2 * page));
}

ZTEST_USER(posix_memory_protection, test_mprotect)
{
	mprotect_invalid();
	mprotect_transitions();
	mprotect_partial();
}

ZTEST_USER(posix_memory_protection, test_mprotect_fault)
{
	volatile uint8_t *p;

	/* an MMU protects the kernel's own view too; an MPU only protects user threads */
	if (IS_ENABLED(CONFIG_NATIVE_LIBC) ||
	    (!IS_ENABLED(CONFIG_MMU) && !IS_ENABLED(CONFIG_USERSPACE))) {
		ztest_test_skip();
	}

	p = mmap(NULL, page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);
	p[0] = 1;
	zassert_ok(mprotect((void *)p, page, PROT_READ));
	zassert_equal(p[0], 1);

	ztest_set_fault_valid(true);
	p[0] = 2;
	ztest_test_fail();
}

static void *setup(void)
{
	long sz = sysconf(_SC_PAGESIZE);

	zassert_true(sz > 0);
	page = (size_t)sz;

	return NULL;
}

ZTEST_SUITE(posix_memory_protection, NULL, setup, NULL, NULL, NULL);
