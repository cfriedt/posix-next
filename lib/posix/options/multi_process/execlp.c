/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <string.h>

#include "multi_process_internal.h"

int execlp(const char *file, const char *arg0, ...)
{
	va_list ap;
	char *argv[CONFIG_POSIX_EXEC_ARGS_MAX + 1];
	int ret;

	va_start(ap, arg0);
	ret = z_posix_execl_argv(argv, arg0, ap);
	va_end(ap);
	if (ret < 0) {
		errno = E2BIG;
		return -1;
	}

	char resolved[CONFIG_POSIX_EXEC_PATH_MAX];

	return execve(z_posix_exec_resolve(file, resolved, sizeof(resolved)), argv, NULL);
}
