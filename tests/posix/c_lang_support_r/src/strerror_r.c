/*
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lang_support_r, test_strerror_r)
{
	char buf[64];
	int ret;

	if (!IS_ENABLED(CONFIG_MINIMAL_LIBC_STRING_ERROR_TABLE)) {
		ztest_test_skip();
		return;
	}

	errno = 4242;
	ret = strerror_r(EINVAL, buf, sizeof(buf));
	zassert_ok(ret, "strerror_r(EINVAL) returned %d", ret);
	zassert_ok(strcmp("Invalid argument", buf), "unexpected message: '%s'", buf);
	zassert_equal(4242, errno, "errno changed on success");

	ret = strerror_r(0, buf, sizeof(buf));
	zassert_ok(ret, "strerror_r(0) returned %d", ret);
	zassert_ok(strcmp("Success", buf), "unexpected message: '%s'", buf);
	zassert_equal(4242, errno, "errno changed on success");

	ret = strerror_r(-42, buf, sizeof(buf));
	zassert_equal(EINVAL, ret, "strerror_r(-42) returned %d", ret);

	ret = strerror_r(4242, buf, sizeof(buf));
	zassert_equal(EINVAL, ret, "strerror_r(4242) returned %d", ret);

	ret = strerror_r(EINVAL, buf, 1);
	zassert_equal(ERANGE, ret, "strerror_r(EINVAL) with small buf returned %d", ret);
}

ZTEST(posix_c_lang_support_r, test_strerror_r_no_table)
{
	char buf[64];
	int ret;

	if (IS_ENABLED(CONFIG_MINIMAL_LIBC_STRING_ERROR_TABLE)) {
		ztest_test_skip();
		return;
	}

	errno = 4242;
	ret = strerror_r(EINVAL, buf, sizeof(buf));
	zassert_ok(ret, "strerror_r(EINVAL) returned %d", ret);
	zassert_ok(strcmp("", buf), "unexpected message: '%s'", buf);
	zassert_equal(4242, errno, "errno changed on success");

	ret = strerror_r(-42, buf, sizeof(buf));
	zassert_equal(EINVAL, ret, "strerror_r(-42) returned %d", ret);

	ret = strerror_r(4242, buf, sizeof(buf));
	zassert_equal(EINVAL, ret, "strerror_r(4242) returned %d", ret);
}
