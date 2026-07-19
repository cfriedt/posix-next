/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>
#include <sys/socket.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_getnetbyname)
{
	struct netent *ne;
	struct netent *ref;

	setnetent(1);
	ref = getnetent();
	zassume_not_null(ref, "no networks in /etc/networks");
	endnetent();

	ne = getnetbyname(ref->n_name);
	zassert_not_null(ne);
	zassert_str_equal(ne->n_name, ref->n_name);
	zassert_equal(ne->n_net, ref->n_net);
}

ZTEST(posix_networking, test_getnetbyaddr)
{
	struct netent *ne;
	struct netent *ref;

	setnetent(1);
	ref = getnetent();
	zassume_not_null(ref, "no networks in /etc/networks");
	endnetent();

	ne = getnetbyaddr(ref->n_net, AF_INET);
	zassert_not_null(ne);
	zassert_str_equal(ne->n_name, ref->n_name);
}

ZTEST(posix_networking, test_getnetent)
{
	struct netent *ne;

	setnetent(1);
	ne = getnetent();
	zassert_not_null(ne);
	endnetent();
}

ZTEST(posix_networking, test_setnetent)
{
	setnetent(0);
	zassert_not_null(getnetent());
	endnetent();
}

ZTEST(posix_networking, test_endnetent)
{
	setnetent(1);
	zassert_not_null(getnetent());
	endnetent();
}

#if !defined(CONFIG_NATIVE_LIBC)
ZTEST(posix_networking, test_getnetbyname_loopback_fallback)
{
	struct netent *ne;

	/*
	 * /etc/networks omits loopback; scan_networks() uses the built-in
	 * fallback table in netdb_ent.c.
	 */
	ne = getnetbyname("loopback");
	zassert_not_null(ne);
	zassert_str_equal(ne->n_name, "loopback");
	zassert_equal(ne->n_net, 127U);
}
#endif /* !CONFIG_NATIVE_LIBC */
