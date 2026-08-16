/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

int execvp(const char *file, char *const argv[])
{
	return execve(file, argv, NULL);
}
