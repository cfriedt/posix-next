/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <spawn.h>

/*
 * Before exec exists there is no PATH search: registry names are exact, so
 * posix_spawnp() and posix_spawn() resolve identically.
 */
int posix_spawnp(pid_t *pid, const char *file, const posix_spawn_file_actions_t *file_actions,
		 const posix_spawnattr_t *attrp, char *const argv[], char *const envp[])
{
	return posix_spawn(pid, file, file_actions, attrp, argv, envp);
}
