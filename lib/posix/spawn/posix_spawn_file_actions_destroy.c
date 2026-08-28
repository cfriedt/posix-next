/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "spawn_internal.h"

#include <errno.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>

int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions)
{
	if (file_actions == NULL) {
		return EINVAL;
	}

	for (int i = 0; i < file_actions->num; i++) {
		free(file_actions->actions[i].path);
	}
	free(file_actions->actions);
	memset(file_actions, 0, sizeof(*file_actions));

	return 0;
}
