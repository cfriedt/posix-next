/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/* the host lookup of POSIX.1-2001: <netdb.h> declares it to a program asking for the extensions */
#define _DEFAULT_SOURCE 1

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_gethostbyaddr)
{
	struct in_addr loopback, unnamed;
	struct hostent *h;

	zassert_equal(inet_pton(AF_INET, "127.0.0.1", &loopback), 1);

	/* the hosts database names the loopback address */
	h = gethostbyaddr(&loopback, sizeof(loopback), AF_INET);
	zassert_not_null(h, "gethostbyaddr: %s", hstrerror(h_errno));
	zassert_str_equal(h->h_name, "localhost");
	zassert_equal(h->h_addrtype, AF_INET);
	zassert_equal(h->h_length, (int)sizeof(struct in_addr));
	zassert_not_null(h->h_aliases);
	zassert_not_null(h->h_addr_list[0]);
	zassert_is_null(h->h_addr_list[1]);
	zassert_mem_equal(h->h_addr_list[0], &loopback, sizeof(loopback));

	/* an address no database names: NULL, and h_errno says why */
	h_errno = 0;
	zassert_equal(inet_pton(AF_INET, "203.0.113.1", &unnamed), 1);
	if (!IS_ENABLED(CONFIG_POSIX_TEST_LINUX_COMPAT)) {
		/* the host's resolver would ask the network */
		zassert_is_null(gethostbyaddr(&unnamed, sizeof(unnamed), AF_INET));
		zassert_equal(h_errno, HOST_NOT_FOUND, "h_errno %d", h_errno);
	}

	/* a length that fits neither address family */
	h_errno = 0;
	zassert_is_null(gethostbyaddr(&loopback, 3, AF_INET));
	zassert_not_equal(h_errno, 0);
}
