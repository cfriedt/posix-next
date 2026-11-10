/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <libgen.h>
#include <string.h>

/* the identifier is renamed by <libgen.h>; the symbol is basename */
char *basename(char *path)
{
	static const char dot[] = ".";
	size_t len;
	char *p;

	if ((path == NULL) || (path[0] == '\0')) {
		return (char *)dot;
	}

	len = strlen(path);
	while ((len > 1) && (path[len - 1] == '/')) {
		path[--len] = '\0';
	}

	if ((len == 1) && (path[0] == '/')) {
		return path;
	}

	p = strrchr(path, '/');

	return (p == NULL) ? path : p + 1;
}
