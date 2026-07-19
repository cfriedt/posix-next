/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arpa/inet.h>
#include <netinet/in.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_htonl)
{
	zassert_equal(htonl(0x01020304U), 0x04030201U);
	zassert_equal(htonl(0U), 0U);
	zassert_equal(htonl(0xffffffffU), 0xffffffffU);
}

ZTEST(posix_networking, test_htons)
{
	zassert_equal(htons(0x1234), 0x3412);
	zassert_equal(htons(0), 0);
	zassert_equal(htons(0xffff), 0xffff);
}

ZTEST(posix_networking, test_ntohl)
{
	zassert_equal(ntohl(0x04030201U), 0x01020304U);
	zassert_equal(ntohl(0U), 0U);
}

ZTEST(posix_networking, test_ntohs)
{
	zassert_equal(ntohs(0x3412), 0x1234);
	zassert_equal(ntohs(0), 0);
}
