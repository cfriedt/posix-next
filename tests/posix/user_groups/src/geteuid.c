/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "user_groups_tests.h"

#include <unistd.h>

ZTEST_USER(posix_user_groups, test_geteuid)
{
	/* the effective identity of a process that never changed it is its real one */
	zassert_equal(geteuid(), getuid());
	IF_NOT_NATIVE_LIBC(zassert_equal(geteuid(), 0));
}
