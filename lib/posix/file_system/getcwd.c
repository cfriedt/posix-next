/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>

char *getcwd(char *buf, size_t size)
{
	char *alloc = NULL;

	if (buf == NULL) {
		/* the allocating form: a common extension, not POSIX */
		if (size == 0) {
			size = PATH_MAX;
		}
		alloc = malloc(size);
		if (alloc == NULL) {
			errno = ENOMEM;
			return NULL;
		}
		buf = alloc;
	}

	if (zvfs_getcwd(buf, size) < 0) {
		free(alloc);
		return NULL;
	}

	return buf;
}
