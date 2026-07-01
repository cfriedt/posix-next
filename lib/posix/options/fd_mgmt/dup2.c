/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/sys/zvfs.h>

int dup2(int fildes, int fildes2)
{
	return zvfs_dup2(fildes, fildes2);
}
