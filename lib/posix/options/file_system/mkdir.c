/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int mkdir(const char *path, mode_t mode)
{
	return zvfs_mkdir(path, mode);
}
