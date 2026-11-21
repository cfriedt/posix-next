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

ZTEST_USER(posix_networking, test_gethostbyname)
{
	struct in_addr loopback;
	struct hostent *h;

	zassert_equal(inet_pton(AF_INET, "127.0.0.1", &loopback), 1);

	/* a numeric address needs no resolver */
	h = gethostbyname("127.0.0.1");
	zassert_not_null(h, "gethostbyname: %s", hstrerror(h_errno));
	zassert_equal(h->h_addrtype, AF_INET);
	zassert_equal(h->h_length, (int)sizeof(struct in_addr));
	zassert_not_null(h->h_name);
	zassert_not_null(h->h_aliases);
	zassert_is_null(h->h_aliases[0]);
	zassert_not_null(h->h_addr_list[0]);
	zassert_is_null(h->h_addr_list[1]);
	zassert_mem_equal(h->h_addr_list[0], &loopback, sizeof(loopback));

	/* a name nothing resolves: NULL, and h_errno says why */
	h_errno = 0;
	zassert_is_null(gethostbyname("no.such.host.invalid"));
	zassert_true((h_errno == HOST_NOT_FOUND) || (h_errno == TRY_AGAIN) ||
			     (h_errno == NO_RECOVERY),
		     "h_errno %d", h_errno);
}
