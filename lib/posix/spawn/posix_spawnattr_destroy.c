/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_destroy(posix_spawnattr_t *attr)
{
	if (attr == NULL) {
		return EINVAL;
	}

	return 0;
}
