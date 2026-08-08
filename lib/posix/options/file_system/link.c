/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/toolchain.h>

int link(const char *path1, const char *path2)
{
	ARG_UNUSED(path1);
	ARG_UNUSED(path2);

	/* the file system subsystem exposes no hard-link operation */
	errno = EPERM;
	return -1;
}
