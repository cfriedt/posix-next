/*
 * Copyright (c) 2024 Google LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/posix/unistd.h>

#include <zephyr/device.h>
#include <zephyr/ztest.h>

#if DT_HAS_CHOSEN(zephyr_entropy)
static const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_entropy));
#else
static const struct device *const dev;
#endif

ZTEST(posix_c_lib_ext, test_getentropy)
{
	/* verify that a buffer larger than 256 bytes returns an error */
	{
		uint8_t buf[256 + 1] = {0};
		int ret;

		ret = getentropy(buf, sizeof(buf));
		zexpect_equal(ret, -1);
		zexpect_equal(errno, EIO);
	}

	if ((dev == NULL) || !device_is_ready(dev)) {
		/* some platforms do not have an entropy device, so skip the tests below */
		ztest_test_skip();
	}

	/* verify that a buffer of 256 bytes returns success */
	{
		uint8_t buf[256] = {0};
		int ret;

		ret = getentropy(buf, sizeof(buf));
		zexpect_equal(ret, 0);
	}

	/* verify that two calls return different data */
	{
		uint8_t zero[16] = {0};
		uint8_t buf1[16];
		uint8_t buf2[16];
		int ret;

		ret = getentropy(buf1, sizeof(buf1));
		zassert_equal(ret, 0);

		ret = getentropy(buf2, sizeof(buf2));
		zassert_equal(ret, 0);

		zassert_true(memcmp(buf1, zero, sizeof(zero)) != 0);
		zassert_true(memcmp(buf2, zero, sizeof(zero)) != 0);
		zassert_true(memcmp(buf1, buf2, sizeof(buf1)) != 0);
	}
}
