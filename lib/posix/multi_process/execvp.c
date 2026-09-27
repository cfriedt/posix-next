/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <unistd.h>

#include "multi_process_internal.h"

int execvp(const char *file, char *const argv[])
{
	char resolved[CONFIG_POSIX_EXEC_PATH_MAX];

	return execve(z_posix_exec_resolve(file, resolved, sizeof(resolved)), argv, NULL);
}
