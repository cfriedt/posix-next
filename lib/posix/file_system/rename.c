/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/sys/zvfs_fs.h>

int rename(const char *old, const char *new)
{
	return zvfs_rename(old, new);
}
