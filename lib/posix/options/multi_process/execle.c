/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/toolchain.h>

int execle(const char *path, const char *arg0, ...)
{
	ARG_UNUSED(path); ARG_UNUSED(arg0);

	errno = ENOSYS;

	return -1;
}
