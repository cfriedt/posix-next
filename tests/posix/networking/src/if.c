/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#include <net/if.h>
#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"

#if !defined(CONFIG_NATIVE_LIBC)
#include <zephyr/net/net_if.h>
#endif

ZTEST(posix_networking, test_if_indextoname)
{
	struct if_nameindex *ni = if_nameindex();

	zassert_not_null(ni, "if_nameindex failed: %d", errno);

	for (size_t i = 0; ni[i].if_index != 0; i++) {
		char buf[IF_NAMESIZE];
		char *name = if_indextoname(ni[i].if_index, buf);

		zassert_not_null(name, "if_indextoname(%u) failed: %d", ni[i].if_index, errno);
		zassert_equal(name, buf);
		zassert_str_equal(buf, ni[i].if_name);

		IF_NOT_NATIVE_LIBC({
			struct net_if *iface = net_if_get_by_index(ni[i].if_index);
			char zname[IF_NAMESIZE];

			zassert_not_null(iface);
			memset(zname, 0, sizeof(zname));
			zassert_true(net_if_get_name(iface, zname, IF_NAMESIZE) >= 0);
			zassert_str_equal(buf, zname);
		});
		TC_PRINT("interface %u: %s\n", ni[i].if_index, buf);
	}

	if_freenameindex(ni);
}

ZTEST(posix_networking, test_if_freenameindex)
{
	struct if_nameindex *ni;

	/* POSIX: NULL is a no-op. Host glibc crashes on NULL (linux_compat). */
	IF_NOT_NATIVE_LIBC({
		if_freenameindex(NULL);
	});

	ni = if_nameindex();
	if (ni != NULL) {
		if_freenameindex(ni);
	}
}

ZTEST(posix_networking, test_if_nameindex)
{
	size_t i;
	struct if_nameindex *ni;

	ni = if_nameindex();
	if (ni == NULL) {
		zassert_equal(errno, ENOBUFS);
		return;
	}

	for (i = 0; ni[i].if_index != 0; i++) {
		zassert_true(ni[i].if_index > 0);
		zassert_not_null(ni[i].if_name);
	}

	zassert_equal(0, ni[i].if_index);
	zassert_is_null(ni[i].if_name);

	if_freenameindex(ni);
}

ZTEST(posix_networking, test_if_nametoindex)
{
	struct if_nameindex *ni = if_nameindex();

	zassert_not_null(ni, "if_nameindex failed: %d", errno);

	for (size_t i = 0; ni[i].if_index != 0; i++) {
		zassert_equal(ni[i].if_index, if_nametoindex(ni[i].if_name));
		TC_PRINT("interface %u: %s\n", ni[i].if_index, ni[i].if_name);
	}

	if_freenameindex(ni);
}

ZTEST(posix_networking, test_if_indextoname_invalid)
{
	char buf[IF_NAMESIZE];

	errno = 0;
	zassert_is_null(if_indextoname(9999, buf));
	zassert_equal(errno, ENXIO);
}
