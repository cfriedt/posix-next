/*
 * Copyright (c) 2023 meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#if defined(__linux__)
#define EXPECTED_SYSNAME "Linux"
#elif defined(__ZEPHYR__)
#define EXPECTED_SYSNAME "Zephyr"
#else
#error "Unsupported platform"
#endif

ZTEST(posix_single_process, test_uname)
{
	struct utsname info;

	zassert_ok(uname(&info));
	zassert_ok(strncmp(info.sysname, EXPECTED_SYSNAME, sizeof(info.sysname)),
		"expected: %s actual: %s", EXPECTED_SYSNAME, info.sysname);
	zassert_true(strlen(info.release) > 0);
	zassert_true(strlen(info.version) > 0);
	zassert_true(strlen(info.machine) > 0);
	zassert_true(strlen(info.nodename) > 0);
}
