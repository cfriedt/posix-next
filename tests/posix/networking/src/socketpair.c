/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "test_net.h"

ZTEST(posix_networking, test_socketpair)
{
	int sv[2];
	char buf[8];
	ssize_t n;

	zassert_ok(socketpair(AF_UNIX, SOCK_STREAM, 0, sv));
	zassert_equal(write(sv[0], TEST_MSG, strlen(TEST_MSG)), strlen(TEST_MSG));
	n = read(sv[1], buf, sizeof(buf));
	zassert_equal(n, strlen(TEST_MSG));
	zassert_true(n < sizeof(buf));
	buf[n] = '\0';
	zassert_str_equal(buf, TEST_MSG);
	close(sv[0]);
	close(sv[1]);
}
