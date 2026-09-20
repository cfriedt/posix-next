/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_SHELL_FUNC_SHELL_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_SHELL_FUNC_SHELL_INTERNAL_H_

#include <stddef.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/thread.h>

/* where the application installs the shell */
#define POSIX_SHELL_PATH "/bin/sh"

/* run "sh -c command" as a process, with the descriptor actions applied in the child */
int posix_shell_spawn(const char *command, const struct sys_clone_fd_action *acts, size_t n,
		      k_pid_t *child);
/* wait for the process and return its POSIX wait status, or -1 with errno set */
int posix_shell_wait(k_pid_t child);

#endif /* ZEPHYR_LIB_POSIX_SHELL_FUNC_SHELL_INTERNAL_H_ */
