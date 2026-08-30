/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_tty.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

int ttyname_r(int fildes, char *name, size_t namesize)
{
	static const char tty_name[] = "/dev/console";

	if (posix_tty_check(fildes) < 0) {
		return errno;
	}

	if (namesize < sizeof(tty_name)) {
		return ERANGE;
	}

	(void)strcpy(name, tty_name);

	return 0;
}
