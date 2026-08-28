/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>
#include <string.h>

int posix_spawnattr_init(posix_spawnattr_t *attr)
{
	if (attr == NULL) {
		return EINVAL;
	}

	memset(attr, 0, sizeof(*attr));
	sigemptyset(&attr->sigdefault);
	sigemptyset(&attr->sigmask);

	return 0;
}
