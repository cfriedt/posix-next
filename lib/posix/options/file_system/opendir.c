/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>

#include <zephyr/sys/zvfs_fs.h>

DIR *opendir(const char *dirname)
{
	return zvfs_opendir(dirname);
}
