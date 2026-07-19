/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "test_net.h"

ZTEST(posix_networking, test_socket)
{
	int sock;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0);
	close(sock);

	errno = 0;
	zassert_equal(socket(99, SOCK_STREAM, IPPROTO_TCP), -1);
	zassert_equal(errno, EAFNOSUPPORT);
}

ZTEST(posix_networking, test_bind)
{
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(0x7f000001);
	addr.sin_port = 0;
	zassert_ok(bind(sock, (struct sockaddr *)&addr, sizeof(addr)));
	close(sock);
}

ZTEST(posix_networking, test_listen)
{
	uint16_t port;
	int sock = tcp_listen_socket(&port);

	ARG_UNUSED(port);
	close(sock);
}

ZTEST(posix_networking, test_connect)
{
	uint16_t port;
	int lsock = tcp_listen_socket(&port);
	int csock = tcp_connect_socket(port);

	close(csock);
	close(lsock);
}

ZTEST(posix_networking, test_accept)
{
	uint16_t port;
	int lsock = tcp_listen_socket(&port);
	int csock = tcp_connect_socket(port);
	struct sockaddr_in peer;
	socklen_t peerlen = sizeof(peer);
	int asock;

	asock = accept(lsock, (struct sockaddr *)&peer, &peerlen);
	zassert_true(asock >= 0);
	close(asock);
	close(csock);
	close(lsock);
}

ZTEST(posix_networking, test_send)
{
	uint16_t port;
	int lsock = tcp_listen_socket(&port);
	int csock = tcp_connect_socket(port);
	struct sockaddr_in peer;
	socklen_t peerlen = sizeof(peer);
	int asock;
	ssize_t sent;

	asock = accept(lsock, (struct sockaddr *)&peer, &peerlen);
	sent = send(csock, TEST_MSG, strlen(TEST_MSG), 0);
	zassert_equal(sent, strlen(TEST_MSG));
	close(asock);
	close(csock);
	close(lsock);
}

ZTEST(posix_networking, test_recv)
{
	uint16_t port;
	int lsock = tcp_listen_socket(&port);
	int csock = tcp_connect_socket(port);
	struct sockaddr_in peer;
	socklen_t peerlen = sizeof(peer);
	int asock;
	char buf[16];
	ssize_t recvd;

	asock = accept(lsock, (struct sockaddr *)&peer, &peerlen);
	zassert_equal(send(csock, TEST_MSG, strlen(TEST_MSG), 0), strlen(TEST_MSG));
	recvd = recv(asock, buf, sizeof(buf), 0);
	zassert_equal(recvd, strlen(TEST_MSG));
	zassert_true(recvd < sizeof(buf));
	buf[recvd] = '\0';
	zassert_str_equal(buf, TEST_MSG);
	close(asock);
	close(csock);
	close(lsock);
}

ZTEST(posix_networking, test_shutdown)
{
	uint16_t port;
	int lsock = tcp_listen_socket(&port);
	int csock = tcp_connect_socket(port);
	struct sockaddr_in peer;
	socklen_t peerlen = sizeof(peer);
	int asock;
	char buf[8];

	asock = accept(lsock, (struct sockaddr *)&peer, &peerlen);
	if (shutdown(csock, SHUT_WR) == 0) {
		zassert_equal(recv(asock, buf, sizeof(buf), 0), 0);
	} else {
		zassert_equal(errno, ENOTSUP);
	}
	close(asock);
	close(csock);
	close(lsock);
}

ZTEST(posix_networking, test_getsockname)
{
	uint16_t port;
	int lsock = tcp_listen_socket(&port);
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	zassert_ok(getsockname(lsock, (struct sockaddr *)&addr, &addrlen));
	zassert_equal(addr.sin_family, AF_INET);
	zassert_equal(ntohs(addr.sin_port), port);
	close(lsock);
}

ZTEST(posix_networking, test_getpeername)
{
	uint16_t port;
	int lsock = tcp_listen_socket(&port);
	int csock = tcp_connect_socket(port);
	struct sockaddr_in peer;
	socklen_t peerlen = sizeof(peer);
	int asock;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	asock = accept(lsock, (struct sockaddr *)&peer, &peerlen);
	zassert_ok(getpeername(asock, (struct sockaddr *)&addr, &addrlen));
	zassert_equal(addr.sin_family, AF_INET);
	close(asock);
	close(csock);
	close(lsock);
}
