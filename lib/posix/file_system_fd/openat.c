/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fcntl.h>
#include <stdarg.h>

#include <zephyr/sys/zvfs_fs.h>

int openat(int fd, const char *path, int oflag, ...)
{
	int mode = 0;
	va_list args;

	if ((oflag & O_CREAT) != 0) {
		va_start(args, oflag);
		mode = va_arg(args, int);
		va_end(args);
	}

	return zvfs_openat(fd, path, oflag, mode);
}
