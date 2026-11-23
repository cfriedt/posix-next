/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <zephyr/ztest.h>

/* a NULL result owns nothing; any other finding is an expectation, so the caller still frees */
static void expect_addr(const struct addrinfo *ai, int family, const char *literal)
{
	unsigned char expected[sizeof(struct in6_addr)];
	const void *actual;

	zassert_not_null(ai);
	zexpect_equal(ai->ai_family, family, "family %d", ai->ai_family);
	if (ai->ai_family != family) {
		return;
	}
	zexpect_equal(inet_pton(family, literal, expected), 1);
	if (family == AF_INET) {
		zexpect_equal(ai->ai_addrlen, sizeof(struct sockaddr_in));
		actual = &((const struct sockaddr_in *)ai->ai_addr)->sin_addr;
		zexpect_mem_equal(actual, expected, sizeof(struct in_addr));
	} else {
		zexpect_equal(ai->ai_addrlen, sizeof(struct sockaddr_in6));
		actual = &((const struct sockaddr_in6 *)ai->ai_addr)->sin6_addr;
		zexpect_mem_equal(actual, expected, sizeof(struct in6_addr));
	}
}

/* the hosts database answers for a name it lists, in the family asked for, case-insensitively */
static void getaddrinfo_hosts(void)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM,
	};
	struct addrinfo *res = NULL;
	int ret;

	ret = getaddrinfo("localhost", "53", &hints, &res);
	zassert_ok(ret, "getaddrinfo failed: %d", ret);
	zassert_not_null(res);
	/* POSIX allows more than one result: every one must match */
	for (const struct addrinfo *ai = res; ai != NULL; ai = ai->ai_next) {
		expect_addr(ai, AF_INET, "127.0.0.1");
		zexpect_equal(ntohs(((const struct sockaddr_in *)ai->ai_addr)->sin_port), 53);
		zexpect_equal(ai->ai_socktype, SOCK_DGRAM);
		zexpect_equal(ai->ai_protocol, IPPROTO_UDP);
	}
	freeaddrinfo(res);

	hints.ai_family = AF_INET6;
	ret = getaddrinfo("IP6-LOCALHOST", NULL, &hints, &res);
	zassert_ok(ret, "getaddrinfo failed: %d", ret);
	expect_addr(res, AF_INET6, "::1");
	freeaddrinfo(res);
}

/* a service is a port number; a name is one no database lists */
static void getaddrinfo_service(void)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res = NULL;
	int ret;

	ret = getaddrinfo("localhost", "no-such-service", &hints, &res);
	zexpect_equal(ret, EAI_SERVICE, "getaddrinfo returned %d", ret);
	zexpect_is_null(res);
	freeaddrinfo(res);
}

/* a name the hosts database does not list goes to Zephyr's resolver, from user mode too */
static void getaddrinfo_resolver(void)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res = NULL;
	int ret;

	/* no server answers: the failure is a result code, not the end of the caller */
	ret = getaddrinfo("host.example", "80", &hints, &res);
	zexpect_not_equal(ret, 0, "an unreachable server resolved a name");
	zexpect_is_null(res);
	freeaddrinfo(res);
}

ZTEST_USER(posix_networking, test_getaddrinfo)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res = NULL;
	int ret;

	ret = getaddrinfo("127.0.0.1", "4242", &hints, &res);
	zassert_ok(ret, "getaddrinfo failed: %d", ret);
	expect_addr(res, AF_INET, "127.0.0.1");
	freeaddrinfo(res);
	res = NULL;

	/* an address literal never leaves the caller, a name must not be mistaken for one */
	hints.ai_flags = AI_NUMERICHOST;
	ret = getaddrinfo("localhost", NULL, &hints, &res);
	zexpect_equal(ret, EAI_NONAME, "getaddrinfo returned %d", ret);
	zexpect_is_null(res);
	freeaddrinfo(res);

	getaddrinfo_hosts();
	getaddrinfo_service();
	if (IS_ENABLED(CONFIG_DNS_RESOLVER)) {
		getaddrinfo_resolver();
	}
}

ZTEST_USER(posix_networking, test_freeaddrinfo)
{
	struct addrinfo *res = NULL;

	zassert_ok(getaddrinfo("127.0.0.1", "80", NULL, &res));
	zassert_not_null(res);
	freeaddrinfo(res);
	freeaddrinfo(NULL);
}

ZTEST(posix_networking, test_gai_strerror)
{
	zassert_not_null(gai_strerror(EAI_NONAME));
	zassert_not_null(gai_strerror(EAI_SERVICE));
}
