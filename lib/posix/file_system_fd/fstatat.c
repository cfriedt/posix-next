/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_stat.h"

#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int fstatat(int fd, const char *ZRESTRICT path, struct stat *ZRESTRICT buf, int flag)
{
	struct zvfs_stat zs;

	if (zvfs_statat(fd, path, &zs, flag) < 0) {
		return -1;
	}

	posix_stat_from_zvfs(buf, &zs);

	return 0;
}
