/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/ztest.h>
#include <zephyr/sys/util.h>

#include "../../common/linux_compat_test.h"

#if defined(__x86_64__) || defined(__amd64__)
#define EXPECT_LP64_OFF64_CFLAGS     "-m64"
#define EXPECT_LP64_OFF64_LDFLAGS    "-m64"
#define EXPECT_WIDTH_RESTRICTED_ENVS "POSIX_V7_LP64_OFF64"
#elif defined(__aarch64__) || (defined(__riscv) && (__riscv_xlen == 64))
#define EXPECT_LP64_OFF64_CFLAGS     ""
#define EXPECT_LP64_OFF64_LDFLAGS    ""
#define EXPECT_WIDTH_RESTRICTED_ENVS "POSIX_V7_LP64_OFF64"
#else
#define EXPECT_LP64_OFF64_CFLAGS     ""
#define EXPECT_LP64_OFF64_LDFLAGS    ""
#define EXPECT_WIDTH_RESTRICTED_ENVS ""
#endif

#define EXPECT_PATH   "/bin:/usr/bin"
#define EXPECT_V7_ENV "POSIXLY_CORRECT=1"

static void check_confstr_empty(int name)
{
	char buf[8];
	size_t len;

	errno = 0;
	len = confstr(name, NULL, 0);
	zassert_equal(1, len, "name %d", name);
	zassert_equal(0, errno);

	buf[0] = 0xff;
	zassert_equal(1, confstr(name, buf, sizeof(buf)), "name %d", name);
	zassert_equal('\0', buf[0], "name %d", name);
}

static void _check_confstr_value(int name, const char *expected, char *name_str, int line)
{
	char buf[128];
	size_t len;
	size_t expected_len = strlen(expected) + 1;

	errno = 0;
	len = confstr(name, NULL, 0);
	zassert_equal(expected_len, len, "%s:%d: name %s (%d)", __FILE__, line, name_str, name);
	zassert_equal(0, errno);

	memset(buf, 0xff, sizeof(buf));
	zassert_equal(expected_len, confstr(name, buf, sizeof(buf)), "%s:%d: name %s (%d)",
		      __FILE__, line, name_str, name);
	zassert_mem_equal(expected, buf, expected_len, "%s:%d: name %s (%d)", __FILE__, line,
			  name_str, name);
}
#define check_confstr_value(name, expected) _check_confstr_value(name, expected, #name, __LINE__)

static void check_confstr_truncated(int name, const char *expected, size_t buflen)
{
	char buf[128];
	size_t full_len = strlen(expected) + 1;

	memset(buf, 0xff, sizeof(buf));
	zassert_equal(full_len, confstr(name, buf, buflen), "name %d", name);
	zassert_equal(0, strncmp(expected, buf, buflen - 1), "name %d", name);
	zassert_equal('\0', buf[buflen - 1], "name %d", name);
}

ZTEST(posix_single_process, test_confstr)
{
	char buf[64];
	size_t len;

	/* invalid name */
	{
		struct arg {
			int name;
			char *buf;
			size_t buflen;
		};

		const struct arg arg1s[] = {
			{-1, NULL, 0},          {-1, NULL, sizeof(buf)}, {-1, buf, 0},
			{-1, buf, sizeof(buf)}, {9999, NULL, 0},         {9999, buf, sizeof(buf)},
		};

		ARRAY_FOR_EACH_PTR(arg1s, arg) {
			errno = 0;
			zassert_equal(0, confstr(arg->name, arg->buf, arg->buflen));
			zassert_equal(errno, EINVAL);
		}
	}

	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(0, confstr(_CS_V6_ENV + 1, NULL, 0));
		zassert_equal(errno, EINVAL);
	});

	len = confstr(_CS_PATH, NULL, 0);
	zassert_true(len > 0, "confstr(_CS_PATH) returned %zu", len);

	buf[0] = '\0';
	zassert_equal(len, confstr(_CS_PATH, buf, sizeof(buf)));
	zassert_true(strlen(buf) < sizeof(buf));

	errno = 0;
	buf[0] = 0xff;
	zassert_true(confstr(_CS_PATH, buf, 0) > 0);
	zassert_equal(errno, 0);
	zassert_equal((uint8_t)buf[0], 0xff);

	check_confstr_value(_CS_PATH, EXPECT_PATH);
	check_confstr_truncated(_CS_PATH, EXPECT_PATH, 4);

#if (_POSIX_C_SOURCE == 200809L)
#ifndef _CS_V7_ENV
#error "_CS_V7_ENV must be defined for Issue 7"
#endif
#endif

#if defined(_CS_V7_ENV)
	if (!(IS_ENABLED(CONFIG_ARCH_POSIX) || IS_ENABLED(CONFIG_ARCH_X86))) {
		check_confstr_empty(_CS_POSIX_V7_ILP32_OFF32_CFLAGS);
		check_confstr_empty(_CS_POSIX_V7_ILP32_OFFBIG_CFLAGS);
	}
	check_confstr_empty(_CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS);
	check_confstr_empty(_CS_POSIX_V7_LP64_OFF64_LIBS);

	IF_NOT_NATIVE_LIBC({
		/* missing iny glibc */
		check_confstr_empty(_CS_POSIX_V7_THREADS_CFLAGS);
		check_confstr_empty(_CS_POSIX_V7_THREADS_LDFLAGS);
	});

	check_confstr_value(_CS_POSIX_V7_LP64_OFF64_CFLAGS, EXPECT_LP64_OFF64_CFLAGS);
	check_confstr_value(_CS_POSIX_V7_LP64_OFF64_LDFLAGS, EXPECT_LP64_OFF64_LDFLAGS);
	if (!(IS_ENABLED(CONFIG_ARCH_POSIX) || IS_ENABLED(CONFIG_ARCH_X86))) {
		check_confstr_value(_CS_POSIX_V7_WIDTH_RESTRICTED_ENVS,
				    EXPECT_WIDTH_RESTRICTED_ENVS);
	}
	check_confstr_value(_CS_V7_ENV, EXPECT_V7_ENV);
#endif

	check_confstr_value(_CS_POSIX_V6_LP64_OFF64_CFLAGS, EXPECT_LP64_OFF64_CFLAGS);
	check_confstr_value(_CS_POSIX_V6_LP64_OFF64_LDFLAGS, EXPECT_LP64_OFF64_LDFLAGS);
	if (!(IS_ENABLED(CONFIG_ARCH_POSIX) || IS_ENABLED(CONFIG_ARCH_X86))) {
		check_confstr_value(_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS,
				    EXPECT_WIDTH_RESTRICTED_ENVS);
	}

	IF_NOT_NATIVE_LIBC({
		/* missing iny glibc */
		check_confstr_value(_CS_V6_ENV, EXPECT_V7_ENV);
		check_confstr_empty(_CS_POSIX_V6_ILP32_OFF32_CFLAGS);
	});
}
