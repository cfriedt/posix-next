/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>

#include "file_system_internal.h"

long fpathconf(int fildes, int name)
{
	struct zvfs_stat st;
	struct zvfs_statvfs zv;
	const struct zvfs_statvfs *zvp = NULL;

	if (zvfs_fstat(fildes, &st) < 0) {
		return -1;
	}

	if (zvfs_fstatvfs(fildes, &zv) == 0) {
		zvp = &zv;
	}

	return posix_pathconf_value(name, zvp);
}
