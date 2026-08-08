/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <zephyr/sys/zvfs_fs.h>
#include <zephyr/toolchain.h>

int readdir_r(DIR *ZRESTRICT dirp, struct dirent *ZRESTRICT entry,
	      struct dirent **ZRESTRICT result)
{
	int rc;

#ifndef CONFIG_NATIVE_LIBC
	/* Silence __nonnull warnings for args the spec leaves undefined. */

	if (result == NULL) {
		return EINVAL;
	}

	if (entry == NULL) {
		*result = NULL;
		return EINVAL;
	}

	if (dirp == NULL) {
		*result = NULL;
		return EBADF;
	}
#endif

	rc = zvfs_readdir(dirp->fd, &dirp->ent);
	if (rc < 0) {
		*result = NULL;
		return errno;
	}

	if (rc > 0) {
		/* end of directory */
		*result = NULL;
		return 0;
	}

	entry->d_ino = (ino_t)dirp->ent.d_ino;
	strncpy(entry->d_name, dirp->ent.d_name, sizeof(entry->d_name));
	entry->d_name[sizeof(entry->d_name) - 1] = '\0';
	*result = entry;

	return 0;
}
