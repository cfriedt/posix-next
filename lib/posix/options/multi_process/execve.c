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

#if defined(CONFIG_POSIX_FD_MGMT) && defined(CONFIG_POSIX_DEVICE_IO)
static void exec_close_cloexec(void)
{
	/* descriptors marked FD_CLOEXEC are closed across exec */
	for (int fd = 0; fd < CONFIG_POSIX_OPEN_MAX; fd++) {
		int flags = fcntl(fd, F_GETFD);

		if ((flags >= 0) && ((flags & FD_CLOEXEC) != 0)) {
			(void)close(fd);
		}
	}
}
#else
static void exec_close_cloexec(void)
{
}
#endif /* CONFIG_POSIX_FD_MGMT && CONFIG_POSIX_DEVICE_IO */

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

	z_posix_exec_prepare();

	img->entry((void *)argv, (void *)envp, NULL);

	/* the image's entry returned: exit as if main() returned 0 */
	exit(0);
}

/*
 * Replace the process image in place: abort every other member thread and
 * continue on the calling thread, preserving the process's identity, parent,
 * and group membership. Signal dispositions revert to default and FD_CLOEXEC
 * descriptors are closed, per POSIX. Remaining deviation: the new image runs
 * on the calling thread's existing stack rather than a fresh one.
 */
void z_posix_exec_prepare(void)
{
	(void)k_process_prune();
#ifdef CONFIG_SIGNAL
	exec_reset_signals();
#endif /* CONFIG_SIGNAL */
	exec_close_cloexec();
}
