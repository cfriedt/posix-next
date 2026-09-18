/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_tests.h"

#include <sys/stat.h>

ZTEST_USER(posix_file_attributes, test_umask)
{
	mode_t initial = umask(0027);

	/* the previous mask comes back, and only the file permission bits are kept */
	zassert_equal(umask(0077), 0027);
	zassert_equal(umask(S_ISUID | S_ISGID | S_ISVTX | 0777), 0077);
	zassert_equal(umask(0), 0777);
	zassert_equal(umask(initial), 0);
	zassert_equal(umask(initial), initial);
}
