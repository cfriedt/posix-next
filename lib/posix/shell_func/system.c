/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_internal.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include <zephyr/sys/zvfs_fs.h>

int system(const char *command)
{
	const struct sigaction ignore = {.sa_handler = SIG_IGN};
	struct sigaction old_int;
	struct sigaction old_quit;
	sigset_t chld;
	sigset_t old_mask;
	struct zvfs_stat st;
	k_pid_t child;
	int status;
	int err;

	if (command == NULL) {
		/* non-zero when a shell is available */
		return zvfs_stat(POSIX_SHELL_PATH, &st) == 0;
	}

	/* cloned first, so the shell inherits the caller's dispositions, not the ones below */
	if (posix_shell_spawn(command, NULL, 0, &child) < 0) {
		/* no interpreter to execute: as if the shell exited with 127 */
		return (errno == ENOENT) ? K_WSTATUS_EXITED(127) : -1;
	}

	/* as POSIX requires while the command runs: SIGINT and SIGQUIT ignored, SIGCHLD blocked */
	(void)sigaction(SIGINT, &ignore, &old_int);
	(void)sigaction(SIGQUIT, &ignore, &old_quit);
	(void)sigemptyset(&chld);
	(void)sigaddset(&chld, SIGCHLD);
	(void)pthread_sigmask(SIG_BLOCK, &chld, &old_mask);

	posix_shell_start(child);
	status = posix_shell_wait(child);
	err = errno;

	(void)pthread_sigmask(SIG_SETMASK, &old_mask, NULL);
	(void)sigaction(SIGQUIT, &old_quit, NULL);
	(void)sigaction(SIGINT, &old_int, NULL);
	errno = err;

	return status;
}
