/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/toolchain.h>

int linkat(int fd1, const char *path1, int fd2, const char *path2, int flag)
{
	ARG_UNUSED(fd1);
	ARG_UNUSED(path1);
	ARG_UNUSED(fd2);
	ARG_UNUSED(path2);
	ARG_UNUSED(flag);

	/* the file system subsystem exposes no hard-link operation */
	errno = EPERM;
	return -1;
}
