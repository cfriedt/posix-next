/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/toolchain.h>

int execvp(const char *file, char *const argv[])
{
	ARG_UNUSED(file); ARG_UNUSED(argv);

	errno = ENOSYS;

	return -1;
}
