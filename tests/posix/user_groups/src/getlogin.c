/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "user_groups_tests.h"

#include <string.h>
#include <unistd.h>

ZTEST_USER(posix_user_groups, test_getlogin)
{
	char *login = getlogin();

	if (IS_ENABLED(CONFIG_NATIVE_LIBC) && login == NULL) {
		/* the host has no login session (e.g. a container) */
		ztest_test_skip();
	}

	zassert_not_null(login);
	zassert_true(strlen(login) > 0);
	/* the same name every time */
	zassert_str_equal(getlogin(), login);
	IF_NOT_NATIVE_LIBC(zassert_str_equal(login, "root"));
}
