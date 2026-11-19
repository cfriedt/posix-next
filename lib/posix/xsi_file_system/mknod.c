/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_mode.h"

#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int mknod(const char *path, mode_t mode, dev_t dev)
{
	return zvfs_mknodat(ZVFS_AT_FDCWD, path, posix_mode_to_zvfs(mode), dev);
}
