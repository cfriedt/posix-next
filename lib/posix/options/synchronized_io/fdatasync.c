/*
 * Copyright (c) 2024 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>
#include <zephyr/sys/zvfs.h>

int fdatasync(int fd)
{
	return zvfs_fsync(fd);
}
