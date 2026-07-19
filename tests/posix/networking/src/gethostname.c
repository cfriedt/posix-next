/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"

ZTEST(posix_networking, test_gethostname)
{
	char hostname[CONFIG_NET_HOSTNAME_MAX_LEN + 1];
	int ret;

	ret = gethostname(hostname, sizeof(hostname));
	zassert_equal(ret, 0, "gethostname() failed: %d", ret);

	IF_NOT_NATIVE_LIBC({
		zassert_equal(strcmp(hostname, CONFIG_NET_HOSTNAME), 0, "unexpected hostname: %s",
			      hostname);
	});

	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		zassert_true(strlen(hostname) > 0, "empty hostname");
	}
}
