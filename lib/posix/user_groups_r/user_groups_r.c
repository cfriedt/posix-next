/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>

int getlogin_r(char *name, size_t namesize)
{
	const char *login = getlogin();

	if (namesize < strlen(login) + 1) {
		return ERANGE;
	}

	(void)strcpy(name, login);

	return 0;
}
