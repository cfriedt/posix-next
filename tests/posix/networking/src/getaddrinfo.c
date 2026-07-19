/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>
#include <sys/socket.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_getaddrinfo)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res = NULL;
	int ret;

	ret = getaddrinfo("127.0.0.1", "4242", &hints, &res);
	zassert_ok(ret, "getaddrinfo failed: %d", ret);
	zassert_not_null(res);
	zassert_equal(res->ai_family, AF_INET);

	freeaddrinfo(res);
	res = NULL;
}

ZTEST(posix_networking, test_freeaddrinfo)
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
