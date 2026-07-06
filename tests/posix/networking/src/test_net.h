/*
 * Copyright (c) 2024 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTS_POSIX_NETWORKING_TEST_NET_H_
#define TESTS_POSIX_NETWORKING_TEST_NET_H_

#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#define TEST_LOOPBACK     "127.0.0.1"
#define TEST_LOOPBACK_V6  "::1"
#define TEST_MSG          "posix"

/* linux_compat: glibc may omit these unless _DEFAULT_SOURCE / __USE_MISC is set. */
#ifndef NI_MAXHOST
#define NI_MAXHOST 64
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV 32
#endif

static inline void prepare_sockaddr_in(struct sockaddr_in *addr, uint16_t port)
{
	memset(addr, 0, sizeof(*addr));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	zassert_equal(1, inet_pton(AF_INET, TEST_LOOPBACK, &addr->sin_addr));
}

static inline int tcp_listen_socket(uint16_t *bound_port)
{
	int sock;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0, "socket failed: %d", errno);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(0x7f000001);
	addr.sin_port = 0;

	zassert_ok(bind(sock, (struct sockaddr *)&addr, sizeof(addr)));
	zassert_ok(listen(sock, 1));
	zassert_ok(getsockname(sock, (struct sockaddr *)&addr, &addrlen));
	*bound_port = ntohs(addr.sin_port);

	return sock;
}

static inline int tcp_connect_socket(uint16_t port)
{
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0, "socket failed: %d", errno);

	prepare_sockaddr_in(&addr, port);
	zassert_ok(connect(sock, (struct sockaddr *)&addr, sizeof(addr)));

	return sock;
}

static inline void prepare_sockaddr_in6(struct sockaddr_in6 *addr, uint16_t port)
{
	memset(addr, 0, sizeof(*addr));
	addr->sin6_family = AF_INET6;
	addr->sin6_port = htons(port);
	zassert_equal(1, inet_pton(AF_INET6, TEST_LOOPBACK_V6, &addr->sin6_addr));
}

#endif /* TESTS_POSIX_NETWORKING_TEST_NET_H_ */
