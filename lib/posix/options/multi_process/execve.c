/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/internal/fdtable_priv.h>

#include "multi_process_internal.h"
#include "posix_image.h"

#ifdef CONFIG_SIGNAL
static void exec_reset_signals(void)
{
	/* handled dispositions revert to default across exec (POSIX) */
	for (int sig = 1; sig < SIGNAL_SET_SIZE; sig++) {
		struct k_sig_action act = {.handler = K_SIG_DFL};
		struct k_sig_action old;

		if (k_sig_action(sig, NULL, &old) != 0) {
			continue;
		}
		if ((old.handler != K_SIG_DFL) && (old.handler != K_SIG_IGN)) {
			(void)k_sig_action(sig, &act, NULL);
		}
	}
}
#endif /* CONFIG_SIGNAL */

static void exec_close_cloexec(void)
{
	/* descriptors marked FD_CLOEXEC in the caller's own table are closed */
	zvfs_fds_cloexec();
}

int execve(const char *path, char *const argv[], char *const envp[])
{
	const struct posix_spawn_image *img;

	if (path == NULL) {
		errno = ENOENT;
		return -1;
	}

	img = posix_spawn_image_lookup(path);
	if ((img == NULL) || (img->entry == NULL)) {
#ifdef CONFIG_POSIX_EXEC_LLEXT
		/* not a prelinked image: try loading an ELF extension */
		return z_posix_exec_llext(path, argv, envp);
#else
		errno = ENOENT;
		return -1;
#endif /* CONFIG_POSIX_EXEC_LLEXT */
	}

	struct z_posix_exec_run_args run = {
		.entry = img->entry,
		.argv = argv,
		.envp = envp,
	};

	if (z_posix_exec_args_check(argv, envp) != 0) {
		return -1;
	}

	z_posix_exec_run(&run);
	CODE_UNREACHABLE;
}

/*
 * Replace the process image: abort every other member thread, preserving the
 * process's identity, parent, and group membership. Signal dispositions
 * revert to default and FD_CLOEXEC descriptors are closed, per POSIX. The
 * new image then runs on a fresh pool-drawn stack (the caller's own stack
 * when the pool is exhausted or CONFIG_SYS_THREAD is absent).
 */
void z_posix_exec_prepare(void)
{
	(void)k_process_prune();
#ifdef CONFIG_SIGNAL
	exec_reset_signals();
#endif /* CONFIG_SIGNAL */
	exec_close_cloexec();
}
