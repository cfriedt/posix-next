/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_getflags(const posix_spawnattr_t *attr, short *flags)
{
	if ((attr == NULL) || (flags == NULL)) {
		return EINVAL;
	}

	*flags = attr->flags;

	return 0;
}
