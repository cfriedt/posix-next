/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_internal.h"

#include <errno.h>
#include <stdlib.h>

#include <zephyr/sys/zvfs_fs.h>

int system(const char *command)
{
	k_pid_t child;
	struct zvfs_stat st;

	if (command == NULL) {
		/* non-zero when a shell is available */
		return zvfs_stat(POSIX_SHELL_PATH, &st) == 0;
	}

	if (posix_shell_spawn(command, NULL, 0, &child) < 0) {
		/* no interpreter to execute: as if the shell exited with 127 */
		return (errno == ENOENT) ? K_WSTATUS_EXITED(127) : -1;
	}

	return posix_shell_wait(child);
}
