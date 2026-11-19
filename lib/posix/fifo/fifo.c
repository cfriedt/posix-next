/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_mode.h"

#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int mkfifo(const char *path, mode_t mode)
{
	return zvfs_mknodat(ZVFS_AT_FDCWD, path, posix_mode_to_zvfs(S_IFIFO | (mode & ~S_IFMT)), 0);
}
