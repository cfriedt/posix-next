/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arpa/inet.h>
#include <netdb.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_getservbyname)
{
	struct servent *se;

	se = getservbyname("http", "tcp");
	zassert_not_null(se);
	zassert_equal(ntohs(se->s_port), 80);
}

ZTEST(posix_networking, test_getservbyport)
{
	struct servent *se;

	se = getservbyport(htons(80), "tcp");
	zassert_not_null(se);
	zassert_str_equal(se->s_name, "http");
}

ZTEST(posix_networking, test_getservent)
{
	struct servent *se;

	setservent(1);
	se = getservent();
	zassert_not_null(se);
	endservent();
}

ZTEST(posix_networking, test_setservent)
{
	setservent(0);
	zassert_not_null(getservent());
	endservent();
}

ZTEST(posix_networking, test_endservent)
{
	setservent(1);
	zassert_not_null(getservent());
	endservent();
}
