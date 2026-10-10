/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

#include <sys/mman.h>
#include <zephyr/sys/zvfs.h>

int mlock(const void *addr, size_t len)
{
	return zvfs_mlock(addr, len, 0);
}

int munlock(const void *addr, size_t len)
{
	return zvfs_munlock(addr, len, 0);
}
