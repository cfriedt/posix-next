/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_getsigmask(const posix_spawnattr_t *attr, sigset_t *sigmask)
{
	if ((attr == NULL) || (sigmask == NULL)) {
		return EINVAL;
	}

	*sigmask = attr->sigmask;

	return 0;
}
