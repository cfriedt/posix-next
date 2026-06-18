/*
 * Copyright (c) 2025 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <zephyr/sys/util.h>

int strncasecmp(const char *s1, const char *s2, size_t n);

int strcasecmp(const char *s1, const char *s2)
{
	return strncasecmp(s1, s2, max(strlen(s1), strlen(s2)));
}
