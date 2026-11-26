/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/sys/zvfs_fs.h>

int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath)
{
	return zvfs_renameat(olddirfd, oldpath, newdirfd, newpath);
}
