/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "advisory_info_tests.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

ZTEST_USER(posix_advisory_info, test_posix_memalign)
{
	static const size_t alignments[] = {sizeof(void *), 16, 64, 256, 4096};
	void *p;

	ARRAY_FOR_EACH(alignments, i) {
		p = NULL;
		zassert_ok(posix_memalign(&p, alignments[i], 100));
		zassert_not_null(p);
		zassert_equal((uintptr_t)p % alignments[i], 0, "%p not aligned to %zu", p,
			      alignments[i]);
		memset(p, 0x5a, 100);
		free(p);
	}

	/* a power of two that is a multiple of the pointer size, or nothing */
	zassert_equal(posix_memalign(&p, 3, 100), EINVAL);
	zassert_equal(posix_memalign(&p, sizeof(void *) / 2, 100), EINVAL);
	zassert_equal(posix_memalign(&p, 0, 100), EINVAL);
	zassert_equal(posix_memalign(&p, 16, SIZE_MAX - 1024), ENOMEM);
}
