/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs_fs.h>
#include <zephyr/toolchain.h>

/* d_type is the type bits of the mode shifted down: the values every system uses */
BUILD_ASSERT((ZVFS_MODE_IFIFO >> 12) == DT_FIFO);
BUILD_ASSERT((ZVFS_MODE_IFCHR >> 12) == DT_CHR);
BUILD_ASSERT((ZVFS_MODE_IFDIR >> 12) == DT_DIR);
BUILD_ASSERT((ZVFS_MODE_IFBLK >> 12) == DT_BLK);
BUILD_ASSERT((ZVFS_MODE_IFREG >> 12) == DT_REG);
BUILD_ASSERT((ZVFS_MODE_IFLNK >> 12) == DT_LNK);
BUILD_ASSERT((ZVFS_MODE_IFSOCK >> 12) == DT_SOCK);

/* readdir() converts in place inside the stream's ZVFS entry storage */
BUILD_ASSERT(sizeof(struct dirent) <= sizeof(struct zvfs_dirent),
	     "struct dirent must fit within struct zvfs_dirent");
BUILD_ASSERT(sizeof(((struct dirent *)0)->d_name) <=
	     sizeof(((struct zvfs_dirent *)0)->d_name),
	     "d_name capacity exceeds the ZVFS entry name capacity");

struct dirent *readdir(DIR *dirp)
{
	int rc;
	ino_t ino;
	unsigned char type;
	struct dirent *entry;

	if (dirp == NULL) {
		errno = EBADF;
		return NULL;
	}

	rc = zvfs_readdir(dirp->fd, &dirp->ent);
	if (rc < 0) {
		return NULL;
	}

	if (rc > 0) {
		/* end of directory: errno intentionally untouched */
		return NULL;
	}

	entry = (struct dirent *)&dirp->ent;
	ino = (ino_t)dirp->ent.d_ino;
	type = (unsigned char)((dirp->ent.d_type & ZVFS_MODE_IFMT) >> 12);
	memmove(entry->d_name, dirp->ent.d_name, strlen(dirp->ent.d_name) + 1);
	entry->d_ino = ino;
	entry->d_type = type;

	return entry;
}
