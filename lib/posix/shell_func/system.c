/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_internal.h"

#include <stdlib.h>

int system(const char *command)
{
	k_pid_t child;

	if (command == NULL) {
		/* a shell is available */
		return 1;
	}

	if (posix_shell_spawn(command, NULL, 0, &child) < 0) {
		return -1;
	}

	return posix_shell_wait(child);
}
