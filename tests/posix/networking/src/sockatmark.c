/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"

ZTEST(posix_networking, test_sockatmark)
{
	int sock;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0);

	errno = 0;
	IF_NOT_NATIVE_LIBC({
		zassert_equal(sockatmark(sock), -1);
		zassert_equal(errno, ENOSYS);
	});

	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		zassert_equal(sockatmark(sock), 0);
	}

	close(sock);

	errno = 0;
	zassert_equal(sockatmark(-1), -1);
	zassert_equal(errno, EBADF);
}
