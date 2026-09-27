/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include "multi_process_internal.h"

int execle(const char *path, const char *arg0, ...)
{
	va_list ap;
	char *argv[CONFIG_POSIX_EXEC_ARGS_MAX + 1];
	char *const *envp;
	int ret;

	va_start(ap, arg0);
	ret = z_posix_execl_argv(argv, arg0, ap);
	if (ret < 0) {
		va_end(ap);
		errno = E2BIG;
		return -1;
	}
	/* the envp pointer follows the argv terminator */
	envp = va_arg(ap, char *const *);
	va_end(ap);

	return execve(path, argv, envp);
}
