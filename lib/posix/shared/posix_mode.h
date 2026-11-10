/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_SHARED_POSIX_MODE_H_
#define ZEPHYR_LIB_POSIX_SHARED_POSIX_MODE_H_

#include <sys/stat.h>

#include <zephyr/sys/fdtable.h>

/* the ZVFS form of a POSIX file mode: the type, then each permission bit */
static inline uint32_t posix_mode_to_zvfs(mode_t mode)
{
	static const struct {
		mode_t posix;
		uint32_t zvfs;
	} bits[] = {
		{S_IRUSR, ZVFS_S_IRUSR}, {S_IWUSR, ZVFS_S_IWUSR}, {S_IXUSR, ZVFS_S_IXUSR},
		{S_IRGRP, ZVFS_S_IRGRP}, {S_IWGRP, ZVFS_S_IWGRP}, {S_IXGRP, ZVFS_S_IXGRP},
		{S_IROTH, ZVFS_S_IROTH}, {S_IWOTH, ZVFS_S_IWOTH}, {S_IXOTH, ZVFS_S_IXOTH},
	};
	uint32_t zmode;

	switch (mode & S_IFMT) {
	case 0:
		zmode = 0;
		break;
	case S_IFREG:
		zmode = ZVFS_MODE_IFREG;
		break;
	case S_IFDIR:
		zmode = ZVFS_MODE_IFDIR;
		break;
	case S_IFIFO:
		zmode = ZVFS_MODE_IFIFO;
		break;
	case S_IFCHR:
		zmode = ZVFS_MODE_IFCHR;
		break;
	case S_IFBLK:
		zmode = ZVFS_MODE_IFBLK;
		break;
	case S_IFLNK:
		zmode = ZVFS_MODE_IFLNK;
		break;
	default:
		/* a type ZVFS has no name for: passed through so the callee refuses it */
		zmode = ZVFS_MODE_IFMT;
		break;
	}

	for (size_t i = 0; i < ARRAY_SIZE(bits); i++) {
		if ((mode & bits[i].posix) != 0) {
			zmode |= bits[i].zvfs;
		}
	}

	return zmode;
}

#endif /* ZEPHYR_LIB_POSIX_SHARED_POSIX_MODE_H_ */
