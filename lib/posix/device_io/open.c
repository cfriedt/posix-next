/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdarg.h>

#include <fcntl.h>
#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>

int open(const char *name, int flags, ...)
{
	int mode = 0;
	va_list args;

	if ((flags & O_CREAT) != 0) {
		va_start(args, flags);
		mode = va_arg(args, int);
		va_end(args);
	}

	return zvfs_open(name, flags, mode);
}
#ifdef CONFIG_POSIX_DEVICE_IO_ALIAS_OPEN
FUNC_ALIAS(open, _open, int);
#endif /* CONFIG_POSIX_DEVICE_IO_ALIAS_OPEN */
