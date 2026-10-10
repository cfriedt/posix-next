/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/* without demand paging every page is resident already: locking succeeds without effect */
static void expect_memlock(int ret)
{
	zassert_ok(ret, "errno %d", errno);
}

/* the host locks a whole process against RLIMIT_MEMLOCK: it may decline */
static void expect_memlock_all(int ret)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC) && (ret != 0)) {
		zassert_true((errno == ENOMEM) || (errno == EPERM), "errno %d", errno);
		return;
	}
	expect_memlock(ret);
}

/* ASAN replaces the host memlock calls with no-ops that succeed */
static bool memlock_is_intercepted(void)
{
	return IS_ENABLED(CONFIG_ASAN) && IS_ENABLED(CONFIG_NATIVE_LIBC);
}

/* kernel memory: user threads cannot read it */
static int page_out_of_reach;

static void *map_page(size_t *page)
{
	long ps = sysconf(_SC_PAGESIZE);
	void *p;

	zassert_true(ps > 0);
	*page = (size_t)ps;
	p = mmap(NULL, *page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	zassert_not_equal(p, MAP_FAILED, "mmap() failed: %d", errno);

	return p;
}

ZTEST_USER(xsi_realtime, test_mlock)
{
	uint8_t *p;
	size_t page;

	if (!memlock_is_intercepted()) {
		/* a wrapping range: Linux reports it as memory it does not have */
		errno = 0;
		zassert_equal(mlock((void *)UINTPTR_MAX, 2), -1);
		zassert_equal(errno, IS_ENABLED(CONFIG_NATIVE_LIBC) ? ENOMEM : EINVAL);
	}
	if (k_is_user_context()) {
		/* memory the caller cannot read is not its to lock */
		zassert_equal(mlock(&page_out_of_reach, sizeof(page_out_of_reach)), -1);
		zassert_equal(errno, ENOMEM);
	}

	p = map_page(&page);
	p[0] = 1;
	expect_memlock(mlock(p, 0));
	expect_memlock(mlock(p, page));
	zassert_equal(p[0], 1);
	expect_memlock(munlock(p, page));
	zassert_ok(munmap(p, page));
}

ZTEST_USER(xsi_realtime, test_munlock)
{
	uint8_t *p;
	size_t page;

	if (!memlock_is_intercepted()) {
		errno = 0;
		zassert_equal(munlock((void *)UINTPTR_MAX, 2), -1);
		zassert_equal(errno, IS_ENABLED(CONFIG_NATIVE_LIBC) ? ENOMEM : EINVAL);
	}

	/* unlocking what was never locked is not an error */
	p = map_page(&page);
	p[0] = 2;
	expect_memlock(munlock(p, 0));
	expect_memlock(munlock(p, page));
	zassert_equal(p[0], 2);
	zassert_ok(munmap(p, page));
}

ZTEST_USER(xsi_realtime, test_mlockall)
{
	uint8_t *p;
	size_t page;

	if (!memlock_is_intercepted()) {
		errno = 0;
		zassert_equal(mlockall(0), -1);
		zassert_equal(errno, EINVAL);
		zassert_equal(mlockall(~(MCL_CURRENT | MCL_FUTURE)), -1);
		zassert_equal(errno, EINVAL);
	}

	expect_memlock_all(mlockall(MCL_CURRENT));
	/* mappings created from now on are locked */
	expect_memlock_all(mlockall(MCL_FUTURE));
	p = map_page(&page);
	p[0] = 3;
	zassert_equal(p[0], 3);
	expect_memlock_all(munlockall());
	zassert_ok(munmap(p, page));
}

ZTEST_USER(xsi_realtime, test_munlockall)
{
	expect_memlock_all(munlockall());
	expect_memlock_all(mlockall(MCL_CURRENT | MCL_FUTURE));
	expect_memlock_all(munlockall());
}
