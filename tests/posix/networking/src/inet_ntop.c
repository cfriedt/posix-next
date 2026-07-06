/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_inet_ntop)
{
	struct in_addr in4;
	char buf[INET_ADDRSTRLEN];

	zassert_equal(1, inet_pton(AF_INET, "127.0.0.1", &in4));
	zassert_not_null(inet_ntop(AF_INET, &in4, buf, sizeof(buf)));
	zassert_mem_equal(buf, "127.0.0.1", strlen("127.0.0.1") + 1);

	zassert_is_null(inet_ntop(AF_INET, &in4, buf, 2));

	zassert_is_null(inet_ntop(AF_UNSPEC, &in4, buf, sizeof(buf)));
}

ZTEST(posix_networking, test_inet_pton)
{
	struct in_addr in4;
	struct in6_addr in6;

	zassert_equal(1, inet_pton(AF_INET, "127.0.0.1", &in4));
	zassert_equal(in4.s_addr, htonl(0x7f000001));

	zassert_equal(0, inet_pton(AF_INET, "not-an-address", &in4));
	zassert_equal(1, inet_pton(AF_INET6, "::1", &in6));
}
