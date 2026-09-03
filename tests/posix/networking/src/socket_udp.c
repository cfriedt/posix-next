/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "test_net.h"

#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY   0

struct icmp_echo {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t seq;
};

static uint16_t inet_checksum(const void *data, size_t len)
{
	const uint8_t *p = data;
	uint32_t sum = 0;

	for (; len > 1; len -= 2, p += 2) {
		sum += (uint16_t)((p[0] << 8) | p[1]);
	}

	if (len > 0) {
		sum += (uint16_t)(p[0] << 8);
	}

	while ((sum >> 16) != 0) {
		sum = (sum & 0xffff) + (sum >> 16);
	}

	return htons((uint16_t)~sum);
}

static int raw_icmp_socket(void)
{
	int sock;
	struct timeval tv = {.tv_sec = 1};

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	zassert_true(sock >= 0, "socket failed: %d", errno);
	zassert_ok(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)));

	return sock;
}

static ssize_t send_icmp_echo_request(int sock, uint16_t id, uint16_t seq)
{
	struct sockaddr_in addr;
	struct icmp_echo echo = {
		.type = ICMP_ECHO_REQUEST,
		.id = htons(id),
		.seq = htons(seq),
	};

	echo.checksum = inet_checksum(&echo, sizeof(echo));
	prepare_sockaddr_in(&addr, 0);

	/* Only the ICMP message is supplied; the stack builds the IPv4 header */
	return sendto(sock, &echo, sizeof(echo), 0, (struct sockaddr *)&addr, sizeof(addr));
}

static void sendto_raw_icmp(void)
{
	int sock;

	sock = raw_icmp_socket();
	zassert_equal(send_icmp_echo_request(sock, 0x1234, 1), sizeof(struct icmp_echo),
		      "sendto failed: %d", errno);
	close(sock);
}

static void recvfrom_raw_icmp(void)
{
	int sock;
	struct sockaddr_in from;
	socklen_t fromlen;
	struct icmp_echo reply = {0};
	uint8_t buf[64];
	ssize_t recvd;
	size_t hlen;

	sock = raw_icmp_socket();
	zassert_equal(send_icmp_echo_request(sock, 0x4321, 7), sizeof(struct icmp_echo),
		      "sendto failed: %d", errno);

	/* Raw IPv4 receives carry the IP header; loopback also delivers our own request */
	for (int i = 0; i < 8; ++i) {
		fromlen = sizeof(from);
		recvd = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
		zassert_true(recvd > 0, "recvfrom failed: %d", errno);
		hlen = (buf[0] & 0x0f) * 4U;
		zassert_true((size_t)recvd >= hlen + sizeof(reply));
		memcpy(&reply, buf + hlen, sizeof(reply));
		if (reply.type == ICMP_ECHO_REPLY && reply.id == htons(0x4321) &&
		    reply.seq == htons(7)) {
			break;
		}
	}

	zassert_equal(reply.type, ICMP_ECHO_REPLY, "no echo reply received");
	zassert_equal(fromlen, sizeof(from));
	zassert_equal(from.sin_family, AF_INET);
	zassert_equal(from.sin_addr.s_addr, htonl(0x7f000001));
	close(sock);
}

ZTEST_USER(posix_networking, test_sendto)
{
	int sock;
	struct sockaddr_in addr;
	ssize_t sent;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	zassert_true(sock >= 0);
	prepare_sockaddr_in(&addr, 4242);
	sent = sendto(sock, TEST_MSG, strlen(TEST_MSG), 0, (struct sockaddr *)&addr, sizeof(addr));
	zassert_equal(sent, strlen(TEST_MSG));
	close(sock);

	if (IS_ENABLED(CONFIG_NET_SOCKETS_INET_RAW)) {
		sendto_raw_icmp();
	}
}

ZTEST_USER(posix_networking, test_recvfrom)
{
	int rx_sock;
	int tx_sock;
	struct sockaddr_in addr;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	char buf[16];
	ssize_t recvd;

	rx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	zassert_true(rx_sock >= 0);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(0x7f000001);
	addr.sin_port = htons(4243);
	zassert_ok(bind(rx_sock, (struct sockaddr *)&addr, sizeof(addr)));

	tx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	zassert_true(tx_sock >= 0);
	prepare_sockaddr_in(&addr, 4243);
	zassert_equal(sendto(tx_sock, TEST_MSG, strlen(TEST_MSG), 0, (struct sockaddr *)&addr,
			     sizeof(addr)),
		      strlen(TEST_MSG));

	recvd = recvfrom(rx_sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
	zassert_equal(recvd, strlen(TEST_MSG));
	zassert_true(recvd < sizeof(buf));
	buf[recvd] = '\0';
	zassert_str_equal(buf, TEST_MSG);
	close(tx_sock);
	close(rx_sock);

	if (IS_ENABLED(CONFIG_NET_SOCKETS_INET_RAW)) {
		recvfrom_raw_icmp();
	}
}
