/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <zephyr/sys/util.h>

int strncasecmp(const char *s1, const char *s2, size_t n);

int strcasecmp(const char *s1, const char *s2)
{
	return strncasecmp(s1, s2, MAX(strlen(s1), strlen(s2)));
}
