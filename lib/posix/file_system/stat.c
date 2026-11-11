/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_stat.h"

#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int stat(const char *ZRESTRICT path, struct stat *ZRESTRICT buf)
{
	struct zvfs_stat zs;

	if (zvfs_stat(path, &zs) < 0) {
		return -1;
	}

	posix_stat_from_zvfs(buf, &zs);

	return 0;
}
