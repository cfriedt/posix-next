/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include "multi_process_internal.h"

int execvp(const char *file, char *const argv[])
{
	char resolved[PATH_MAX];
	const char *path;
	int err;

	path = z_posix_exe_path(file, resolved, sizeof(resolved), &err);
	if (path == NULL) {
		errno = err;
		return -1;
	}

	return execve(path, argv, z_posix_exec_environ());
}
