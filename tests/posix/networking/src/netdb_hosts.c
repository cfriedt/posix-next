/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_gethostent)
{
	struct hostent *he;

	sethostent(1);
	he = gethostent();
	zassert_not_null(he);
	zassert_str_equal(he->h_name, "localhost");
	endhostent();
}

ZTEST(posix_networking, test_sethostent)
{
	sethostent(0);
	zassert_not_null(gethostent());
	endhostent();
}

ZTEST(posix_networking, test_endhostent)
{
	sethostent(1);
	zassert_not_null(gethostent());
	endhostent();
	endhostent();
}
