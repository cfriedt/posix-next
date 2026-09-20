/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"
#include "shell_internal.h"

#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

/* the caller's environment, when a process environment exists at all */
extern char **environ __attribute__((weak));

/* what a shell needs to find its commands when there is no environment to pass on */
static char *const shell_default_env[] = {"PATH=/bin", NULL};

int posix_shell_spawn(const char *command, const struct sys_clone_fd_action *acts, size_t n,
		      k_pid_t *child)
{
	int ret;
	char *const argv[] = {"sh", "-c", (char *)command, NULL};
	struct sys_clone_args args = {
		.flags = SYS_CLONE_PAUSED,
		.prio = k_thread_priority_get(k_current_get()),
		.options = IS_ENABLED(CONFIG_USERSPACE) ? K_USER : 0,
		.argv = argv,
		.envp = ((&environ != NULL) && (environ != NULL)) ? environ : shell_default_env,
		.fd_actions = acts,
		.fd_actions_len = n,
	};

	ret = sys_clone(&args, child);
	if (ret < 0) {
		errno = (ret == -E2BIG || ret == -EINVAL) ? -ret : EAGAIN;
		return -1;
	}

	ret = sys_exec_load(*child, POSIX_SHELL_PATH);
	if (ret < 0) {
		/* a failed spawn leaves no child: abort the paused leader and reap it */
		k_thread_abort(*child);
		(void)k_waitpid(*child, NULL, NULL, 0, K_FOREVER);
		errno = (ret == -ENOMEM) ? ENOMEM : ENOENT;
		return -1;
	}

	k_thread_start(*child);

	return 0;
}

int posix_shell_wait(k_pid_t child)
{
	int kws = 0;
	int ret = k_waitpid(child, NULL, &kws, 0, K_FOREVER);

	if (ret < 0) {
		errno = ECHILD;
		return -1;
	}

	/* the layout is shared; a killing signal is renumbered */
	if (K_WIFSIGNALED(kws)) {
		return (kws & ~0x7f) | (z_sig_to_posix(K_WTERMSIG(kws)) & 0x7f);
	}

	return kws;
}
