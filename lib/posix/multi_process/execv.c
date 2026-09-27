/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include "multi_process_internal.h"

int execv(const char *path, char *const argv[])
{
	return execve(path, argv, z_posix_exec_environ());
}
