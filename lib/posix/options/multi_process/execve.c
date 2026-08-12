/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/toolchain.h>

int execve(const char *path, char *const argv[], char *const envp[])
{
	ARG_UNUSED(path); ARG_UNUSED(argv); ARG_UNUSED(envp);

	errno = ENOSYS;

	return -1;
}
