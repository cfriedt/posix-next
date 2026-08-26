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

struct posix_spawn_file_action *posix_spawn_file_actions_grow(posix_spawn_file_actions_t *fa);

int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions, int fildes)
{
	struct posix_spawn_file_action *act;

	if (file_actions == NULL) {
		return EINVAL;
	}
	if (fildes < 0) {
		return EBADF;
	}

	act = posix_spawn_file_actions_grow(file_actions);
	if (act == NULL) {
		return ENOMEM;
	}

	*act = (struct posix_spawn_file_action){
		.type = POSIX_SPAWN_FILE_ACTION_CLOSE,
		.fildes = fildes,
	};
	file_actions->num++;

	return 0;
}
