/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_getschedparam(const posix_spawnattr_t *attr, struct sched_param *schedparam)
{
	if ((attr == NULL) || (schedparam == NULL)) {
		return EINVAL;
	}

	*schedparam = attr->schedparam;

	return 0;
}
