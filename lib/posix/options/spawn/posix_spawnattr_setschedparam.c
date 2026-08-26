/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_setschedparam(posix_spawnattr_t *attr, const struct sched_param *schedparam)
{
	if ((attr == NULL) || (schedparam == NULL)) {
		return EINVAL;
	}

	attr->schedparam = *schedparam;

	return 0;
}
