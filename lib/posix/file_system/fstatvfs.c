/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/statvfs.h>

#include <zephyr/sys/zvfs_fs.h>

#include "file_system_internal.h"

int fstatvfs(int fildes, struct statvfs *buf)
{
	struct zvfs_statvfs zv;

	if (zvfs_fstatvfs(fildes, &zv) < 0) {
		return -1;
	}

	statvfs_from_zvfs(buf, &zv);

	return 0;
}
