/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_getsigdefault(const posix_spawnattr_t *attr, sigset_t *sigdefault)
{
	if ((attr == NULL) || (sigdefault == NULL)) {
		return EINVAL;
	}

	*sigdefault = attr->sigdefault;

	return 0;
}
