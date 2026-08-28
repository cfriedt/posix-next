/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/zvfs/pipe.h>

int pipe(int fildes[2])
{
	return zvfs_pipe(fildes, 0);
}
