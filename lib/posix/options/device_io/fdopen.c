/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/sys/zvfs_libc.h>

FILE *fdopen(int fd, const char *mode)
{
	return zvfs_libc_fdopen(fd, mode);
}
#ifdef CONFIG_POSIX_DEVICE_IO_ALIAS_FDOPEN
FUNC_ALIAS(fdopen, _fdopen, FILE *);
#endif /* CONFIG_POSIX_DEVICE_IO_ALIAS_FDOPEN */
