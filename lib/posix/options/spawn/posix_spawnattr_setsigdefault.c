/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_setsigdefault(posix_spawnattr_t *attr, const sigset_t *sigdefault)
{
	if ((attr == NULL) || (sigdefault == NULL)) {
		return EINVAL;
	}

	attr->sigdefault = *sigdefault;

	return 0;
}
