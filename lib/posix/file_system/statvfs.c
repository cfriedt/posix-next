/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/statvfs.h>

#include <zephyr/sys/zvfs_fs.h>

#include "file_system_internal.h"

int statvfs(const char *ZRESTRICT path, struct statvfs *ZRESTRICT buf)
{
	struct zvfs_statvfs zv;

	if (zvfs_statvfs(path, &zv) < 0) {
		return -1;
	}

	statvfs_from_zvfs(buf, &zv);

	return 0;
}
