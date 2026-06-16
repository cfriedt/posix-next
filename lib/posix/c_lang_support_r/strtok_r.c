/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <string.h>

char *strtok_r(char *str, const char *sep, char **state)
{
	char *start, *end;

	start = str ? str : *state;

	/* skip leading delimiters */
	while (*start && strchr(sep, *start)) {
		start++;
	}

	if (*start == '\0') {
		*state = start;
		return NULL;
	}

	/* look for token chars */
	end = start;
	while (*end && !strchr(sep, *end)) {
		end++;
	}

	if (*end != '\0') {
		*end = '\0';
		*state = end + 1;
	} else {
		*state = end;
	}

	return start;
}
