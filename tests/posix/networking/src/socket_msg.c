/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../../common/linux_compat_test.h"
#include "test_net.h"

ZTEST(posix_networking, test_sendmsg)
{
	uint16_t port;
	int lsock = tcp_listen_socket(&port);
	int csock = tcp_connect_socket(port);
	struct sockaddr_in peer;
	struct sockaddr_in dest;
	socklen_t peerlen = sizeof(peer);
	int asock;
	struct iovec iov;
	struct msghdr msg;
	ssize_t sent;

	asock = accept(lsock, (struct sockaddr *)&peer, &peerlen);
	zassert_ok(getpeername(csock, (struct sockaddr *)&dest, &peerlen));
	iov.iov_base = TEST_MSG;
	iov.iov_len = strlen(TEST_MSG);
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &dest;
	msg.msg_namelen = sizeof(dest);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	sent = sendmsg(csock, &msg, 0);
	zassert_equal(sent, strlen(TEST_MSG));
	close(asock);
	close(csock);
	close(lsock);
}

ZTEST(posix_networking, test_recvmsg)
{
	int rx_sock;
	int tx_sock;
	struct sockaddr_in addr;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	struct iovec iov;
	struct msghdr msg;
	char buf[16];
	ssize_t recvd;

	rx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	zassert_true(rx_sock >= 0);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(0x7f000001);
	addr.sin_port = htons(4246);
	zassert_ok(bind(rx_sock, (struct sockaddr *)&addr, sizeof(addr)));

	tx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	zassert_true(tx_sock >= 0);
	prepare_sockaddr_in(&addr, 4246);
	zassert_equal(sendto(tx_sock, TEST_MSG, strlen(TEST_MSG), 0, (struct sockaddr *)&addr,
			     sizeof(addr)),
		      strlen(TEST_MSG));

	memset(&from, 0, sizeof(from));
	fromlen = sizeof(from);
	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &from;
	msg.msg_namelen = sizeof(from);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	recvd = recvmsg(rx_sock, &msg, 0);
	zassert_equal(recvd, strlen(TEST_MSG));
	zassert_true(recvd < sizeof(buf));
	buf[recvd] = '\0';
	zassert_str_equal(buf, TEST_MSG);
	zassert_equal(from.sin_family, AF_INET);
	close(tx_sock);
	close(rx_sock);
}

ZTEST(posix_networking, test_recvmsg_null)
{
	IF_NOT_NATIVE_LIBC({
		int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		zassert_true(sock >= 0);
		zassert_equal(recvmsg(sock, NULL, 0), -1);
		zassert_equal(errno, EINVAL);
		close(sock);
	});
}
