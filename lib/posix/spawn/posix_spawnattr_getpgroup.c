/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_getpgroup(const posix_spawnattr_t *attr, pid_t *pgroup)
{
	if ((attr == NULL) || (pgroup == NULL)) {
		return EINVAL;
	}

	*pgroup = attr->pgroup;

	return 0;
}
