/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

ZTEST_SUITE(posix_user_groups_r, NULL, NULL, NULL, NULL, NULL);

ZTEST_USER(posix_user_groups_r, test_getlogin_r)
{
	char buf[64];
	char *login = getlogin();
	int ret;

	if (IS_ENABLED(CONFIG_NATIVE_LIBC) && login == NULL) {
		/* the host has no login session (e.g. a container) */
		ztest_test_skip();
	}
	zassert_not_null(login);
	zassert_true(strlen(login) < sizeof(buf));

	memset(buf, 'x', sizeof(buf));
	ret = getlogin_r(buf, sizeof(buf));
	zassert_equal(ret, 0, "getlogin_r: %d", ret);
	zassert_str_equal(buf, login);

	/* the name and its terminator must fit */
	zassert_equal(getlogin_r(buf, strlen(login) + 1), 0);
	zassert_str_equal(buf, login);
	zassert_equal(getlogin_r(buf, strlen(login)), ERANGE);
	zassert_equal(getlogin_r(buf, 0), ERANGE);
}
