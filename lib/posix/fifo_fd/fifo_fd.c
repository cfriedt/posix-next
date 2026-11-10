/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_mode.h"

#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int mkfifoat(int fd, const char *path, mode_t mode)
{
	return zvfs_mknodat(fd, path, posix_mode_to_zvfs(S_IFIFO | (mode & ~S_IFMT)), 0);
}

int mknodat(int fd, const char *path, mode_t mode, dev_t dev)
{
	return zvfs_mknodat(fd, path, posix_mode_to_zvfs(mode), dev);
}
