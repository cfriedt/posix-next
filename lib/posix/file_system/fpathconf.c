/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>

#include "file_system_internal.h"

static int fpathconf_ioctl(int fildes, unsigned long request, ...)
{
	va_list args;
	int ret;

	va_start(args, request);
	ret = zvfs_ioctl(fildes, request, args);
	va_end(args);

	return ret;
}

long fpathconf(int fildes, int name)
{
	struct zvfs_stat st;
	struct zvfs_statvfs zv;
	const struct zvfs_statvfs *zvp = NULL;

	if (name == _PC_VDISABLE) {
		/* only a terminal has special characters to disable */
		int saved = errno;

		if (fpathconf_ioctl(fildes, ZFD_IOCTL_ISATTY) == 0) {
			return _POSIX_VDISABLE;
		}
		if (errno != EBADF) {
			/* no association: -1 with errno unchanged */
			errno = saved;
		}
		return -1;
	}

	if (zvfs_fstat(fildes, &st) < 0) {
		return -1;
	}

	if (zvfs_fstatvfs(fildes, &zv) == 0) {
		zvp = &zv;
	}

	return posix_pathconf_value(name, zvp);
}
