/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "user_groups_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_user_groups, test_setgid)
{
	gid_t id = getgid();

	/* the identity a process already has is always available to it */
	zassert_ok(setgid(id));
	zassert_equal(getgid(), id);

	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(setgid(TEST_OTHER_ID), -1);
		zassert_equal(errno, EPERM);
		zassert_equal(getgid(), id);
	});
}
