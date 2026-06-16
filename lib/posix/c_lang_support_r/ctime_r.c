/*
 * Copyright (c) 2024 Meta Platforms
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>

char *ctime_r(const time_t *clock, char *buf)
{
	struct tm tmp;

	return asctime_r(localtime_r(clock, &tmp), buf);
}
