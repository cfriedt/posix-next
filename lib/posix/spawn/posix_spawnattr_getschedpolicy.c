/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *attr, int *schedpolicy)
{
	if ((attr == NULL) || (schedpolicy == NULL)) {
		return EINVAL;
	}

	*schedpolicy = attr->schedpolicy;

	return 0;
}
