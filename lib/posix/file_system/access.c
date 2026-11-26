/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/sys/fdtable.h>

#include "posix_access.h"

int access(const char *path, int amode)
{
	return posix_access_check(ZVFS_AT_FDCWD, path, amode, 0);
}
