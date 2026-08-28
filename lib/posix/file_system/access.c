/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>
#include <zephyr/toolchain.h>

BUILD_ASSERT(F_OK == ZVFS_F_OK && R_OK == ZVFS_R_OK && W_OK == ZVFS_W_OK && X_OK == ZVFS_X_OK,
	     "POSIX amode constants must match their ZVFS counterparts");

int access(const char *path, int amode)
{
	return zvfs_access(path, amode);
}
