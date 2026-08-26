/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_setsigmask(posix_spawnattr_t *attr, const sigset_t *sigmask)
{
	if ((attr == NULL) || (sigmask == NULL)) {
		return EINVAL;
	}

	attr->sigmask = *sigmask;

	return 0;
}
