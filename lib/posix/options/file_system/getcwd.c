/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>

char *getcwd(char *buf, size_t size)
{
	if (buf == NULL) {
		/* the allocating form is a common extension, not POSIX */
		errno = EINVAL;
		return NULL;
	}

	if (zvfs_getcwd(buf, size) < 0) {
		return NULL;
	}

	return buf;
}
