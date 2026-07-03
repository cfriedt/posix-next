/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_getsockopt)
{
	int sock;
	int optval;
	socklen_t optlen = sizeof(optval);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0);
	zassert_ok(getsockopt(sock, SOL_SOCKET, SO_TYPE, &optval, &optlen));
	zassert_equal(optval, SOCK_STREAM);
	close(sock);
}

ZTEST(posix_networking, test_setsockopt)
{
	int sock;
	int optval = 1;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0);
	zassert_ok(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)));
	close(sock);
}
