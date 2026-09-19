/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "advisory_info_tests.h"

#include <errno.h>
#include <sys/mman.h>

/* the host wants page-aligned regions */
static ZTEST_BMEM __aligned(4096) char region[4096];

ZTEST_USER(posix_advisory_info, test_posix_madvise)
{
	static const int advice[] = {POSIX_MADV_NORMAL, POSIX_MADV_RANDOM, POSIX_MADV_SEQUENTIAL,
				     POSIX_MADV_WILLNEED, POSIX_MADV_DONTNEED};

	ARRAY_FOR_EACH(advice, i) {
		zassert_ok(posix_madvise(region, sizeof(region), advice[i]));
		zassert_ok(posix_madvise(region, 0, advice[i]));
	}

	zassert_equal(posix_madvise(region, sizeof(region), -1), EINVAL);
	zassert_equal(posix_madvise(region, sizeof(region), 4242), EINVAL);
}
