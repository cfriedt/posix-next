/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_mode.h"

#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int mkfifo(const char *path, mode_t mode)
{
	return zvfs_mknod(path, posix_mode_to_zvfs(S_IFIFO | (mode & ~S_IFMT)), 0);
}
