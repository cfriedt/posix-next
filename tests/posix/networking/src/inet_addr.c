/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

ZTEST(posix_networking, test_inet_addr)
{
	in_addr_t ret;
	static const struct parm_fail {
		const char *in;
	} fail_parms[] = {
#ifndef CONFIG_ARCH_POSIX
		{NULL},
#endif
		{"."},
		{".."},
		{"..."},
		{"-1.-2.-3.-4"},
		{"256.65536.4294967296.18446744073709551616"},
		{"a.b.c.d"},
		{"0.0.0.1234"},
		{"0.0.0.12a"},
		{" 1.2.3.4"},
	};
	static const struct parm_ok {
		const char *in;
		const char *canonical;
	} ok_parms[] = {
		{"0.0.0.0", "0.0.0.0"},
		{"000.00.0.0", "0.0.0.0"},
		{"127.0.0.1", "127.0.0.1"},
		{"1.2.3.4", "1.2.3.4"},
		{"1.2.3.4    ", "1.2.3.4"},
		{"0.0.0.123 a", "0.0.0.123"},
		{"255.255.255.255", "255.255.255.255"},
	};

	ARRAY_FOR_EACH_PTR(fail_parms, p) {
		ret = inet_addr(p->in);
		zexpect_equal(ret, (in_addr_t)-1, "inet_addr(%s) should fail", p->in);
	}

	ARRAY_FOR_EACH_PTR(ok_parms, p) {
		struct in_addr expected;

		zassert_equal(1, inet_pton(AF_INET, p->canonical, &expected));
		ret = inet_addr(p->in);
		zexpect_equal(ret, expected.s_addr, "inet_addr(%s) failed", p->in);
	}
}
