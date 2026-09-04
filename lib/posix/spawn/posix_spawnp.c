/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits.h>
#include <spawn.h>

#include "posix_internal.h"

int posix_spawnp(pid_t *pid, const char *file, const posix_spawn_file_actions_t *file_actions,
		 const posix_spawnattr_t *attrp, char *const argv[], char *const envp[])
{
	char resolved[PATH_MAX];
	const char *path;
	int err;

	path = z_posix_exe_path(file, resolved, sizeof(resolved), &err);
	if (path == NULL) {
		return err;
	}

	return posix_spawn(pid, path, file_actions, attrp, argv, envp);
}
