/*
 * Copyright (c) 2024 Meta Platforms
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <grp.h>
#include <pwd.h>

#include <zephyr/ztest.h>

ZTEST(posix_system_database_r, test_getpwnam_r)
{
	zassert_equal(getpwnam_r(NULL, NULL, NULL, 42, NULL), ENOSYS);
}

ZTEST(posix_system_database_r, test_getpwuid_r)
{
	zassert_equal(getpwuid_r(42, NULL, NULL, 42, NULL), ENOSYS);
}

ZTEST(posix_system_database_r, test_getgrnam_r)
{
	zassert_equal(getgrnam_r(NULL, NULL, NULL, 42, NULL), ENOSYS);
}

ZTEST(posix_system_database_r, test_getgrgid_r)
{
	zassert_equal(getgrgid_r(42, NULL, NULL, 42, NULL), ENOSYS);
}

ZTEST_SUITE(posix_system_database_r, NULL, NULL, NULL, NULL, NULL);
