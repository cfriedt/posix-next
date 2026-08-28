/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdarg.h>
#include <stddef.h>

#include "multi_process_internal.h"

int z_posix_execl_argv(char **argv, const char *arg0, va_list ap)
{
	size_t n = 0;
	const char *arg = arg0;

	while (arg != NULL) {
		if (n >= CONFIG_POSIX_EXEC_ARGS_MAX) {
			return -1;
		}
		argv[n++] = (char *)arg;
		arg = va_arg(ap, const char *);
	}
	argv[n] = NULL;

	return 0;
}
