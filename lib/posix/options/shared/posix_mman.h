/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_OPTIONS_SHARED_POSIX_MMAN_H_
#define ZEPHYR_LIB_POSIX_OPTIONS_SHARED_POSIX_MMAN_H_

#include <sys/mman.h>

#include <zephyr/sys/zvfs.h>

/* POSIX page protection to ZVFS_PROT_*, or -1 for an unknown bit */
static inline int posix_prot_to_zvfs(int prot)
{
	int zprot = ZVFS_PROT_NONE;

	if ((prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC)) != 0) {
		return -1;
	}
	if ((prot & PROT_READ) != 0) {
		zprot |= ZVFS_PROT_READ;
	}
	if ((prot & PROT_WRITE) != 0) {
		zprot |= ZVFS_PROT_WRITE;
	}
	if ((prot & PROT_EXEC) != 0) {
		zprot |= ZVFS_PROT_EXEC;
	}

	return zprot;
}

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_SHARED_POSIX_MMAN_H_ */
