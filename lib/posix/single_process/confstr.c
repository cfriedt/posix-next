/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#include <unistd.h>

#include <zephyr/sys/util.h>

#define Z_CS_PATH "/bin:/usr/bin"
#define Z_CS_ENV "POSIXLY_CORRECT=1"
#define Z_CS_EMPTY ""

#if defined(__x86_64__) || defined(__amd64__)
#define Z_CS_LP64_OFF64_CFLAGS "-m64"
#define Z_CS_LP64_OFF64_LDFLAGS "-m64"
#define Z_CS_WIDTH_RESTRICTED "POSIX_V7_LP64_OFF64"
#elif defined(__aarch64__) || (defined(__riscv) && (__riscv_xlen == 64))
#define Z_CS_LP64_OFF64_CFLAGS Z_CS_EMPTY
#define Z_CS_LP64_OFF64_LDFLAGS Z_CS_EMPTY
#define Z_CS_WIDTH_RESTRICTED "POSIX_V7_LP64_OFF64"
#else
#define Z_CS_LP64_OFF64_CFLAGS Z_CS_EMPTY
#define Z_CS_LP64_OFF64_LDFLAGS Z_CS_EMPTY
#define Z_CS_WIDTH_RESTRICTED Z_CS_EMPTY
#endif

static size_t confstr_copy(const char *val, char *buf, size_t len)
{
	size_t n = strlen(val) + 1;

	if (buf != NULL && len > 0) {
		size_t copy = MIN(n - 1, len - 1);

		memcpy(buf, val, copy);
		buf[copy] = '\0';
	}

	return n;
}

size_t confstr(int name, char *buf, size_t len)
{
	const char *val;

	switch (name) {
	case _CS_PATH:
		val = Z_CS_PATH;
		break;

	case _CS_POSIX_V7_ILP32_OFF32_CFLAGS:
	case _CS_POSIX_V7_ILP32_OFF32_LDFLAGS:
	case _CS_POSIX_V7_ILP32_OFF32_LIBS:
	case _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS:
	case _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS:
	case _CS_POSIX_V7_ILP32_OFFBIG_LIBS:
	case _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS:
	case _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS:
	case _CS_POSIX_V7_LPBIG_OFFBIG_LIBS:
	case _CS_POSIX_V7_THREADS_CFLAGS:
	case _CS_POSIX_V7_LP64_OFF64_LIBS:
	case _CS_POSIX_V7_THREADS_LDFLAGS:
		val = Z_CS_EMPTY;
		break;

	case _CS_POSIX_V7_LP64_OFF64_CFLAGS:
		val = Z_CS_LP64_OFF64_CFLAGS;
		break;

	case _CS_POSIX_V7_LP64_OFF64_LDFLAGS:
		val = Z_CS_LP64_OFF64_LDFLAGS;
		break;

	case _CS_POSIX_V7_WIDTH_RESTRICTED_ENVS:
		val = Z_CS_WIDTH_RESTRICTED;
		break;

	case _CS_V7_ENV:
		val = Z_CS_ENV;
		break;

#if defined(_CS_V6_ENV) && (_CS_V6_ENV != _CS_V7_ENV)
	case _CS_POSIX_V6_ILP32_OFF32_CFLAGS:
	case _CS_POSIX_V6_ILP32_OFF32_LDFLAGS:
	case _CS_POSIX_V6_ILP32_OFF32_LIBS:
	case _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS:
	case _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS:
	case _CS_POSIX_V6_ILP32_OFFBIG_LIBS:
	case _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS:
	case _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS:
	case _CS_POSIX_V6_LP64_OFF64_LIBS:
	case _CS_POSIX_V6_LPBIG_OFFBIG_LIBS:
		val = Z_CS_EMPTY;
		break;

	case _CS_POSIX_V6_LP64_OFF64_CFLAGS:
		val = Z_CS_LP64_OFF64_CFLAGS;
		break;

	case _CS_POSIX_V6_LP64_OFF64_LDFLAGS:
		val = Z_CS_LP64_OFF64_LDFLAGS;
		break;

	case _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS:
		val = Z_CS_WIDTH_RESTRICTED;
		break;

	case _CS_V6_ENV:
		val = Z_CS_ENV;
		break;
#endif

	default:
		errno = EINVAL;
		return 0;
	}

	return confstr_copy(val, buf, len);
}
