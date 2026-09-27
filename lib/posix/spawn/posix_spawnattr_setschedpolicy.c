/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attr, int schedpolicy)
{
	if (attr == NULL) {
		return EINVAL;
	}

	attr->schedpolicy = schedpolicy;

	return 0;
}
