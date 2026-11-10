/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_SHARED_POSIX_ACCESS_H_
#define ZEPHYR_LIB_POSIX_SHARED_POSIX_ACCESS_H_

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs_fs.h>

#include "posix_identity.h"

/*
 * The file system holding the node: by name when the name resolves from the
 * working directory, else through a descriptor opened on it. A special file
 * is not opened (opening a FIFO connects it); its directory's file system
 * stands in.
 */
static inline int posix_access_statvfs(int dirfd, const char *path, const struct zvfs_stat *st,
				       struct zvfs_statvfs *vfs)
{
	int fd;
	int rc;
	uint32_t type = st->mode & ZVFS_MODE_IFMT;

	if ((dirfd == AT_FDCWD) || (path[0] == '/')) {
		return zvfs_statvfs(path, vfs);
	}
	if ((type != ZVFS_MODE_IFREG) && (type != ZVFS_MODE_IFDIR)) {
		return zvfs_fstatvfs(dirfd, vfs);
	}
	fd = zvfs_openat(dirfd, path, ZVFS_O_RDONLY, 0);
	if (fd < 0) {
		return -1;
	}
	rc = zvfs_fstatvfs(fd, vfs);
	(void)zvfs_close(fd);

	return rc;
}

/*
 * Whether the caller's identity may access a node as amode asks, from the
 * node's recorded mode and owner (ZVFS keeps no identity, so it cannot decide
 * this). The privileged identity reads and writes anything, but runs a file
 * only when some execute bit is set, as Linux does; others take their class's
 * bits. A read-only file system refuses writes to everyone.
 */
static inline int posix_access_check(int dirfd, const char *path, int amode, int flags)
{
	struct zvfs_stat st;
	struct zvfs_statvfs vfs;
	uint32_t bits;

	if (((amode & ~(R_OK | W_OK | X_OK)) != 0) || ((flags & ~AT_EACCESS) != 0)) {
		errno = EINVAL;
		return -1;
	}

	if (zvfs_statat(dirfd, path, &st, 0) < 0) {
		return -1;
	}
	if (amode == F_OK) {
		return 0;
	}

	if ((amode & W_OK) != 0) {
		/* a read-only file system refuses writes to the privileged identity too */
		if ((posix_access_statvfs(dirfd, path, &st, &vfs) == 0) &&
		    ((vfs.f_flag & ZVFS_ST_RDONLY) != 0)) {
			errno = EROFS;
			return -1;
		}
	}

	if (posix_euid() == 0) {
		bits = ZVFS_S_IRWXU | ZVFS_S_IRWXG | ZVFS_S_IRWXO;
		if (((amode & X_OK) != 0) && ((st.mode & bits) & (ZVFS_S_IXUSR | ZVFS_S_IXGRP | ZVFS_S_IXOTH)) == 0) {
			errno = EACCES;
			return -1;
		}
		return 0;
	}

	if (st.uid == posix_euid()) {
		bits = (st.mode & ZVFS_S_IRWXU) >> 6;
	} else if (st.gid == posix_egid()) {
		bits = (st.mode & ZVFS_S_IRWXG) >> 3;
	} else {
		bits = st.mode & ZVFS_S_IRWXO;
	}
	if ((amode & ~bits) != 0) {
		errno = EACCES;
		return -1;
	}

	return 0;
}

#endif /* ZEPHYR_LIB_POSIX_SHARED_POSIX_ACCESS_H_ */
