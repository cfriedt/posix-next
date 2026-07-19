/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_getprotobyname)
{
	struct protoent *pe;

	pe = getprotobyname("tcp");
	zassert_not_null(pe);
	zassert_equal(pe->p_proto, 6);
}

ZTEST(posix_networking, test_getprotobynumber)
{
	struct protoent *pe;

	pe = getprotobynumber(17);
	zassert_not_null(pe);
	zassert_str_equal(pe->p_name, "udp");
}

ZTEST(posix_networking, test_getprotoent)
{
	struct protoent *pe;

	setprotoent(1);
	pe = getprotoent();
	zassert_not_null(pe);
	endprotoent();
}

ZTEST(posix_networking, test_setprotoent)
{
	setprotoent(0);
	zassert_not_null(getprotoent());
	endprotoent();
}

ZTEST(posix_networking, test_endprotoent)
{
	setprotoent(1);
	zassert_not_null(getprotoent());
	endprotoent();
}
