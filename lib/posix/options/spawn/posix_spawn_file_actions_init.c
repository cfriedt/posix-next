/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>
#include <string.h>

int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions)
{
	if (file_actions == NULL) {
		return EINVAL;
	}

	memset(file_actions, 0, sizeof(*file_actions));

	return 0;
}
