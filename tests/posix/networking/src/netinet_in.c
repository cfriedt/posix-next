/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <zephyr/ztest.h>

static void in6(struct in6_addr *addr, const char *str)
{
	zassert_equal(inet_pton(AF_INET6, str, addr), 1, "inet_pton(%s) failed", str);
}

ZTEST_USER(posix_networking, test_in6addr_any)
{
	const struct in6_addr any = IN6ADDR_ANY_INIT;
	struct in6_addr unspec;

	in6(&unspec, "::");

	zassert_mem_equal(&in6addr_any, &any, sizeof(any));
	zassert_mem_equal(&in6addr_any, &unspec, sizeof(unspec));
}

ZTEST_USER(posix_networking, test_in6addr_loopback)
{
	const struct in6_addr lo = IN6ADDR_LOOPBACK_INIT;
	struct in6_addr loopback;

	in6(&loopback, "::1");

	zassert_mem_equal(&in6addr_loopback, &lo, sizeof(lo));
	zassert_mem_equal(&in6addr_loopback, &loopback, sizeof(loopback));
}

ZTEST_USER(posix_networking, test_in6_is_addr_unspecified)
{
	struct in6_addr unspec;
	struct in6_addr loopback;
	struct in6_addr global;

	in6(&unspec, "::");
	in6(&loopback, "::1");
	in6(&global, "2001:db8::1");

	zassert_true(IN6_IS_ADDR_UNSPECIFIED(&unspec));
	zassert_true(IN6_IS_ADDR_UNSPECIFIED(&in6addr_any));
	zassert_false(IN6_IS_ADDR_UNSPECIFIED(&loopback));
	zassert_false(IN6_IS_ADDR_UNSPECIFIED(&global));
}

ZTEST_USER(posix_networking, test_in6_is_addr_loopback)
{
	struct in6_addr unspec;
	struct in6_addr loopback;
	struct in6_addr compat;

	in6(&unspec, "::");
	in6(&loopback, "::1");
	in6(&compat, "::2");

	zassert_true(IN6_IS_ADDR_LOOPBACK(&loopback));
	zassert_true(IN6_IS_ADDR_LOOPBACK(&in6addr_loopback));
	zassert_false(IN6_IS_ADDR_LOOPBACK(&unspec));
	zassert_false(IN6_IS_ADDR_LOOPBACK(&compat));
}

ZTEST_USER(posix_networking, test_in6_is_addr_multicast)
{
	struct in6_addr mc_link;
	struct in6_addr mc_global;
	struct in6_addr ll;
	struct in6_addr global;

	in6(&mc_link, "ff02::1");
	in6(&mc_global, "ff0e::1");
	in6(&ll, "fe80::1");
	in6(&global, "2001:db8::1");

	zassert_true(IN6_IS_ADDR_MULTICAST(&mc_link));
	zassert_true(IN6_IS_ADDR_MULTICAST(&mc_global));
	zassert_false(IN6_IS_ADDR_MULTICAST(&ll));
	zassert_false(IN6_IS_ADDR_MULTICAST(&global));
}

ZTEST_USER(posix_networking, test_in6_is_addr_linklocal)
{
	struct in6_addr ll;
	struct in6_addr ll_high;
	struct in6_addr sl;
	struct in6_addr global;

	in6(&ll, "fe80::1");
	in6(&ll_high, "febf::1");
	in6(&sl, "fec0::1");
	in6(&global, "2001:db8::1");

	zassert_true(IN6_IS_ADDR_LINKLOCAL(&ll));
	zassert_true(IN6_IS_ADDR_LINKLOCAL(&ll_high));
	zassert_false(IN6_IS_ADDR_LINKLOCAL(&sl));
	zassert_false(IN6_IS_ADDR_LINKLOCAL(&global));
}

ZTEST_USER(posix_networking, test_in6_is_addr_sitelocal)
{
	struct in6_addr sl;
	struct in6_addr sl_high;
	struct in6_addr ll;
	struct in6_addr global;

	in6(&sl, "fec0::1");
	in6(&sl_high, "feff::1");
	in6(&ll, "fe80::1");
	in6(&global, "2001:db8::1");

	zassert_true(IN6_IS_ADDR_SITELOCAL(&sl));
	zassert_true(IN6_IS_ADDR_SITELOCAL(&sl_high));
	zassert_false(IN6_IS_ADDR_SITELOCAL(&ll));
	zassert_false(IN6_IS_ADDR_SITELOCAL(&global));
}

ZTEST_USER(posix_networking, test_in6_is_addr_v4mapped)
{
	struct in6_addr mapped;
	struct in6_addr compat;
	struct in6_addr loopback;
	struct in6_addr global;

	in6(&mapped, "::ffff:192.0.2.1");
	in6(&compat, "::192.0.2.1");
	in6(&loopback, "::1");
	in6(&global, "2001:db8::1");

	zassert_true(IN6_IS_ADDR_V4MAPPED(&mapped));
	zassert_false(IN6_IS_ADDR_V4MAPPED(&compat));
	zassert_false(IN6_IS_ADDR_V4MAPPED(&loopback));
	zassert_false(IN6_IS_ADDR_V4MAPPED(&global));
}

ZTEST_USER(posix_networking, test_in6_is_addr_v4compat)
{
	struct in6_addr compat;
	struct in6_addr compat_low;
	struct in6_addr mapped;
	struct in6_addr unspec;
	struct in6_addr loopback;
	struct in6_addr global;

	in6(&compat, "::192.0.2.1");
	in6(&compat_low, "::0.0.0.2");
	in6(&mapped, "::ffff:192.0.2.1");
	in6(&unspec, "::");
	in6(&loopback, "::1");
	in6(&global, "2001:db8::1");

	zassert_true(IN6_IS_ADDR_V4COMPAT(&compat));
	zassert_true(IN6_IS_ADDR_V4COMPAT(&compat_low));
	zassert_false(IN6_IS_ADDR_V4COMPAT(&mapped));
	zassert_false(IN6_IS_ADDR_V4COMPAT(&unspec));
	zassert_false(IN6_IS_ADDR_V4COMPAT(&loopback));
	zassert_false(IN6_IS_ADDR_V4COMPAT(&global));
}

ZTEST_USER(posix_networking, test_in6_is_addr_mc_nodelocal)
{
	struct in6_addr mc_node;
	struct in6_addr mc_link;
	struct in6_addr global;

	in6(&mc_node, "ff01::1");
	in6(&mc_link, "ff02::1");
	in6(&global, "2001:db8::1");

	zassert_true(IN6_IS_ADDR_MC_NODELOCAL(&mc_node));
	zassert_false(IN6_IS_ADDR_MC_NODELOCAL(&mc_link));
	zassert_false(IN6_IS_ADDR_MC_NODELOCAL(&global));
}

ZTEST_USER(posix_networking, test_in6_is_addr_mc_linklocal)
{
	struct in6_addr mc_link;
	struct in6_addr mc_site;
	struct in6_addr ll;

	in6(&mc_link, "ff02::1");
	in6(&mc_site, "ff05::1");
	in6(&ll, "fe80::1");

	zassert_true(IN6_IS_ADDR_MC_LINKLOCAL(&mc_link));
	zassert_false(IN6_IS_ADDR_MC_LINKLOCAL(&mc_site));
	zassert_false(IN6_IS_ADDR_MC_LINKLOCAL(&ll));
}

ZTEST_USER(posix_networking, test_in6_is_addr_mc_sitelocal)
{
	struct in6_addr mc_site;
	struct in6_addr mc_org;
	struct in6_addr sl;

	in6(&mc_site, "ff05::1");
	in6(&mc_org, "ff08::1");
	in6(&sl, "fec0::1");

	zassert_true(IN6_IS_ADDR_MC_SITELOCAL(&mc_site));
	zassert_false(IN6_IS_ADDR_MC_SITELOCAL(&mc_org));
	zassert_false(IN6_IS_ADDR_MC_SITELOCAL(&sl));
}

ZTEST_USER(posix_networking, test_in6_is_addr_mc_orglocal)
{
	struct in6_addr mc_org;
	struct in6_addr mc_global;
	struct in6_addr global;

	in6(&mc_org, "ff08::1");
	in6(&mc_global, "ff0e::1");
	in6(&global, "2001:db8::1");

	zassert_true(IN6_IS_ADDR_MC_ORGLOCAL(&mc_org));
	zassert_false(IN6_IS_ADDR_MC_ORGLOCAL(&mc_global));
	zassert_false(IN6_IS_ADDR_MC_ORGLOCAL(&global));
}

ZTEST_USER(posix_networking, test_in6_is_addr_mc_global)
{
	struct in6_addr mc_global;
	struct in6_addr mc_link;
	struct in6_addr global;

	in6(&mc_global, "ff0e::1");
	in6(&mc_link, "ff02::1");
	in6(&global, "2001:db8::1");

	zassert_true(IN6_IS_ADDR_MC_GLOBAL(&mc_global));
	zassert_false(IN6_IS_ADDR_MC_GLOBAL(&mc_link));
	zassert_false(IN6_IS_ADDR_MC_GLOBAL(&global));
}

ZTEST_USER(posix_networking, test_netinet_in_constants)
{
	struct in_addr any = {.s_addr = INADDR_ANY};
	struct in_addr bcast = {.s_addr = INADDR_BROADCAST};
	char buf[INET6_ADDRSTRLEN];

	zassert_equal(ntohl(any.s_addr), 0x00000000U);
	zassert_equal(ntohl(bcast.s_addr), 0xffffffffU);

	/* the string buffer sizes hold the longest textual form plus the terminator */
	zassert_true(INET_ADDRSTRLEN >= sizeof("255.255.255.255"));
	zassert_true(INET6_ADDRSTRLEN >= sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"));
	zassert_not_null(inet_ntop(AF_INET, &bcast, buf, sizeof(buf)));
	zassert_equal(strcmp(buf, "255.255.255.255"), 0);

	/* protocol numbers are the IANA assignments */
	zassert_equal(IPPROTO_IP, 0);
	zassert_equal(IPPROTO_ICMP, 1);
	zassert_equal(IPPROTO_TCP, 6);
	zassert_equal(IPPROTO_UDP, 17);
	zassert_equal(IPPROTO_IPV6, 41);
	zassert_equal(IPPROTO_RAW, 255);
}
