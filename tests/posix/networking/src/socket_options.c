/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_networking, test_getsockopt)
{
	int sock;
	int optval;
	socklen_t optlen = sizeof(optval);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0);
	zassert_ok(getsockopt(sock, SOL_SOCKET, SO_TYPE, &optval, &optlen));
	zassert_equal(optval, SOCK_STREAM);
	/* a negative length is invalid whatever the option */
	optlen = (socklen_t)-1;
	zassert_equal(getsockopt(sock, SOL_SOCKET, SO_TYPE, &optval, &optlen), -1);
	zassert_equal(errno, EINVAL);
	close(sock);
}

ZTEST_USER(posix_networking, test_setsockopt)
{
	int sock;
	int optval = 1;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0);
	zassert_ok(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)));
	/* a negative length is invalid whatever the option */
	zassert_equal(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)-1), -1);
	zassert_equal(errno, EINVAL);
	close(sock);
}
