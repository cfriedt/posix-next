/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <unistd.h>

#include "multi_process_internal.h"

int execlp(const char *file, const char *arg0, ...)
{
	va_list ap;
	char *argv[CONFIG_POSIX_EXEC_ARGS_MAX + 1];
	char resolved[PATH_MAX];
	const char *path;
	int err;
	int ret;

	va_start(ap, arg0);
	ret = z_posix_execl_argv(argv, arg0, ap);
	va_end(ap);
	if (ret < 0) {
		errno = E2BIG;
		return -1;
	}

	path = z_posix_exe_path(file, resolved, sizeof(resolved), &err);
	if (path == NULL) {
		errno = err;
		return -1;
	}

	return execve(path, argv, z_posix_exec_environ());
}
