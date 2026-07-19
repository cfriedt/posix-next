/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <zephyr/ztest.h>

#include "test_net.h"

ZTEST(posix_networking, test_getnameinfo)
{
	struct sockaddr_in addr;
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	socklen_t addrlen = sizeof(addr);
	uint16_t port;
	int lsock;

	lsock = tcp_listen_socket(&port);
	zassert_ok(getsockname(lsock, (struct sockaddr *)&addr, &addrlen));
	close(lsock);

	zassert_ok(getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof(host), serv,
			       sizeof(serv), NI_NUMERICHOST | NI_NUMERICSERV));
	zassert_mem_equal(host, TEST_LOOPBACK, strlen(TEST_LOOPBACK) + 1);
}

ZTEST(posix_networking, test_getnameinfo_ipv6)
{
	struct sockaddr_in6 addr;
	char host[NI_MAXHOST];
	socklen_t addrlen = sizeof(addr);

	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	zassert_equal(1, inet_pton(AF_INET6, TEST_LOOPBACK_V6, &addr.sin6_addr));

	zassert_ok(getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof(host), NULL, 0,
			       NI_NUMERICHOST));
	zassert_true(strcmp(host, "::1") == 0 || strcmp(host, "0:0:0:0:0:0:0:1") == 0);
}
