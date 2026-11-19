/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "user_groups_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_user_groups, test_seteuid)
{
	uid_t id = geteuid();

	/* the identity a process already has is always available to it */
	zassert_ok(seteuid(id));
	zassert_equal(geteuid(), id);

	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(seteuid(TEST_OTHER_ID), -1);
		zassert_equal(errno, EPERM);
		zassert_equal(geteuid(), id);
	});
}
