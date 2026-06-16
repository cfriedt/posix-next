/*
 * Copyright (c) 2021 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#define OUTPUT_BITS (0x7fffffffU)
#define MULTIPLIER (1103515245U)
#define INCREMENT (12345U)

int rand_r(unsigned int *seed)
{
	*seed = (MULTIPLIER * *seed + INCREMENT) & OUTPUT_BITS;

	return *seed;
}
