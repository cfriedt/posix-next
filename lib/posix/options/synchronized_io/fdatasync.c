/*
 * Copyright (c) 2024 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

/* prototypes for external, not-yet-public, functions in fdtable.c */
int zvfs_fsync(int fd);

int fdatasync(int fd)
{
	return zvfs_fsync(fd);
}
