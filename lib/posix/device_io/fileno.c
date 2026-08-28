/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/sys/zvfs_libc.h>

int fileno(FILE *file)
{
	return zvfs_libc_fileno(file);
}
#ifdef CONFIG_POSIX_DEVICE_IO_ALIAS_FILENO
FUNC_ALIAS(fileno, _fileno, int);
#endif /* CONFIG_POSIX_DEVICE_IO_ALIAS_FILENO */
