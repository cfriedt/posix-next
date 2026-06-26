/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_LINUX_COMPAT_TEST_H_
#define POSIX_TESTS_LINUX_COMPAT_TEST_H_

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

/*
 * linux_compat twister variants run with CONFIG_NATIVE_LIBC on native_sim.
 *
 * Host pthread APIs annotate pointer parameters __nonnull__; passing NULL in
 * EINVAL degenerate-case tests fails -Werror=nonnull even in dead code.
 * Wrap those checks in IF_NOT_NATIVE_LIBC({ ... }).
 *
 * Tests that only apply to the Zephyr pthread implementation should call
 * posix_test_skip_if_native_libc() at entry.
 */

#if defined(CONFIG_NATIVE_LIBC)
#define IF_NOT_NATIVE_LIBC(...)
#else
#define IF_NOT_NATIVE_LIBC(...) __VA_ARGS__
#endif

static inline void posix_test_skip_if_native_libc(void)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		ztest_test_skip();
	}
}

int msleep(int ms);
uint32_t now_ms(void);

#endif /* POSIX_TESTS_LINUX_COMPAT_TEST_H_ */
