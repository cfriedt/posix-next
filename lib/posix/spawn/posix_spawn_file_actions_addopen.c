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

struct posix_spawn_file_action *posix_spawn_file_actions_grow(posix_spawn_file_actions_t *fa)
{
	if (fa->num == fa->cap) {
		int cap = (fa->cap == 0) ? 4 : (fa->cap * 2);
		struct posix_spawn_file_action *acts =
			realloc(fa->actions, (size_t)cap * sizeof(*acts));

		if (acts == NULL) {
			return NULL;
		}
		fa->actions = acts;
		fa->cap = cap;
	}

	return &fa->actions[fa->num];
}

int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *file_actions, int fildes,
				     const char *path, int oflag, mode_t mode)
{
	struct posix_spawn_file_action *act;
	char *copy;

	if ((file_actions == NULL) || (path == NULL)) {
		return EINVAL;
	}
	if (fildes < 0) {
		return EBADF;
	}

	act = posix_spawn_file_actions_grow(file_actions);
	if (act == NULL) {
		return ENOMEM;
	}

	copy = strdup(path);
	if (copy == NULL) {
		return ENOMEM;
	}

	*act = (struct posix_spawn_file_action){
		.type = POSIX_SPAWN_FILE_ACTION_OPEN,
		.fildes = fildes,
		.path = copy,
		.oflag = oflag,
		.mode = mode,
	};
	file_actions->num++;

	return 0;
}
