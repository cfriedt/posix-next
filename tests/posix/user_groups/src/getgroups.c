/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "user_groups_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_user_groups, test_getgroups)
{
	gid_t list[64];
	int count;

	/* a zero size asks for the count */
	count = getgroups(0, NULL);
	zassert_true(count >= 0, "getgroups: %d (errno %d)", count, errno);
	zassert_true(count <= (int)ARRAY_SIZE(list), "%d supplementary groups", count);
	IF_NOT_NATIVE_LIBC(zassert_equal(count, 0));

	zassert_equal(getgroups(ARRAY_SIZE(list), list), count);
	zassert_equal(getgroups(count, list), count);

	if (count > 0) {
		errno = 0;
		zassert_equal(getgroups(count - 1, list), -1);
		zassert_equal(errno, EINVAL);
	}

	/* a negative size, kept from the fortified wrappers that would reject the call */
	volatile int negative = -1;
	gid_t *volatile unsized = list;

	errno = 0;
	zassert_equal(getgroups(negative, unsized), -1);
	zassert_equal(errno, EINVAL);
}
