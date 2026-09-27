/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/zvfs/shm.h>

/* a shared memory object name is a slash followed by at least one character */
static bool shm_name_valid(const char *name)
{
	return (name != NULL) && (name[0] == '/') && (name[1] != '\0') &&
	       (strnlen(name, PATH_MAX) < PATH_MAX);
}

int shm_open(const char *name, int oflag, mode_t mode)
{
	int acc = oflag & O_ACCMODE;

	/* revisit when file-based permissions are available */
	if ((mode & 0777) == 0) {
		errno = EINVAL;
		return -1;
	}

	if ((acc != O_RDONLY) && (acc != O_RDWR)) {
		errno = EINVAL;
		return -1;
	}

	if ((acc == O_RDONLY) && ((oflag & O_TRUNC) != 0)) {
		errno = EINVAL;
		return -1;
	}

	if (!shm_name_valid(name)) {
		errno = EINVAL;
		return -1;
	}

	return zvfs_shm_open(name, oflag & (O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC), mode);
}

int shm_unlink(const char *name)
{
	if (!shm_name_valid(name)) {
		errno = EINVAL;
		return -1;
	}

	return zvfs_shm_unlink(name);
}
