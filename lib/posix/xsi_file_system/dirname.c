/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <libgen.h>
#include <string.h>

char *dirname(char *path)
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

	p = strrchr(path, '/');
	if (p == NULL) {
		return (char *)dot;
	}

	/* drop the last component and the slashes before it */
	while ((p > path) && (*p == '/')) {
		p--;
	}

	if ((p == path) && (*p == '/')) {
		path[1] = '\0';
	} else {
		p[1] = '\0';
	}

	return path;
}
