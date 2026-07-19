/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "test_net.h"

ZTEST(posix_networking, test_sendto_ipv6)
{
	int sock;
	struct sockaddr_in6 addr;
	ssize_t sent;

	sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	zassert_true(sock >= 0);
	prepare_sockaddr_in6(&addr, 4244);
	sent = sendto(sock, TEST_MSG, strlen(TEST_MSG), 0, (struct sockaddr *)&addr, sizeof(addr));
	zassert_equal(sent, strlen(TEST_MSG));
	close(sock);
}

ZTEST(posix_networking, test_recvfrom_ipv6)
{
	int rx_sock;
	int tx_sock;
	struct sockaddr_in6 addr;
	struct sockaddr_in6 from;
	socklen_t fromlen = sizeof(from);
	char buf[16];
	ssize_t recvd;

	rx_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	zassert_true(rx_sock >= 0);
	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	zassert_equal(1, inet_pton(AF_INET6, TEST_LOOPBACK_V6, &addr.sin6_addr));
	addr.sin6_port = htons(4245);
	zassert_ok(bind(rx_sock, (struct sockaddr *)&addr, sizeof(addr)));

	tx_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	zassert_true(tx_sock >= 0);
	prepare_sockaddr_in6(&addr, 4245);
	zassert_equal(sendto(tx_sock, TEST_MSG, strlen(TEST_MSG), 0, (struct sockaddr *)&addr,
			     sizeof(addr)),
		      strlen(TEST_MSG));

	recvd = recvfrom(rx_sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
	zassert_equal(recvd, strlen(TEST_MSG));
	zassert_true(recvd < sizeof(buf));
	buf[recvd] = '\0';
	zassert_str_equal(buf, TEST_MSG);
	zassert_equal(from.sin6_family, AF_INET6);
	close(tx_sock);
	close(rx_sock);
}
