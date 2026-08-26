/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags)
{
	if (attr == NULL) {
		return EINVAL;
	}

	attr->flags = flags;

	return 0;
}
