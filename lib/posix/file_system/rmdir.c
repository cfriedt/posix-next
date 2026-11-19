/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>

int rmdir(const char *path)
{
	return zvfs_unlinkat(ZVFS_AT_FDCWD, path, ZVFS_AT_REMOVEDIR);
}
