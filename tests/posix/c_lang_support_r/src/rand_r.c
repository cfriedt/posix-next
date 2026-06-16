/*
 * Copyright (c) 2021 Space Cubics, LLC.
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <zephyr/ztest.h>

ZTEST(posix_c_lang_support_r, test_rand_r)
{
	unsigned int seed;
	unsigned int value;

	struct rand_r_test_data {
		const unsigned int seed;
		/* populated once at the beginning of the test */
		int expected_value;
	} data[] = {
		{ .seed = 0, },
		{ .seed = 1, },
		{ .seed = 10, },
		{ .seed = UINT_MAX - 1, },
		{ .seed = UINT_MAX, },
	};

	/* populate the expectations exactly once */
	ARRAY_FOR_EACH(data, i) {
		seed = data[i].seed;
		data[i].expected_value = rand_r(&seed);
	}

	/* Ensure that different seeds produce different values */
	ARRAY_FOR_EACH(data, i) {
		ARRAY_FOR_EACH(data, j) {
			if (i == j) {
				continue;
			}
			zassert_not_equal(data[i].expected_value, data[j].expected_value,
					  "rand_r(%u) == rand_r(%u)", data[i].seed, data[j].seed);
		}
	}

	/* reproducibility tests */
	for (int i = 0; i < ARRAY_SIZE(data); i++) {
		for (int j = 0; j < 3; j++) {
			seed = data[i].seed;
			value = rand_r(&seed);
			zassert_equal(data[i].expected_value, value, "rand_r(%u) %u != %u",
				      data[i].seed, value, data[i].expected_value);
		}
	}

	/* ensure that seed is updated by calls to rand_r() */
	seed = 42;
	for (int i = 0; i < 100; ++i) {
		/* hijack 'value' to preserve previous seed */
		value = seed;

		(void)rand_r(&seed);
		zassert_not_equal(value, seed, "iter[%d] seed not updated (was %u, is %u)",
				  i, value, seed);
	}
}
