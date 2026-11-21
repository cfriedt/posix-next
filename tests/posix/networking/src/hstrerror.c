/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/* the host lookup of POSIX.1-2001: <netdb.h> declares it to a program asking for the extensions */
#define _DEFAULT_SOURCE 1

#include <netdb.h>
#include <string.h>

#include <zephyr/ztest.h>

ZTEST(posix_networking, test_hstrerror)
{
	const int codes[] = {HOST_NOT_FOUND, TRY_AGAIN, NO_RECOVERY, NO_DATA};

	ARRAY_FOR_EACH(codes, i) {
		const char *s = hstrerror(codes[i]);

		zassert_not_null(s);
		zassert_true(strlen(s) > 0);
		zassert_true(strcmp(s, hstrerror(-1)) != 0, "code %d has no description of its own",
			     codes[i]);
	}
	zassert_not_null(hstrerror(0));
	zassert_not_null(hstrerror(-1));
}
