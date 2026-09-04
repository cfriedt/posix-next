/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/process.h>

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

/* the vectors must fit the kernel's staging budget before anything irreversible */
static int exec_args_check(char *const argv[], char *const envp[])
{
	size_t bytes = 2 * sizeof(char *);

	for (int i = 0; (argv != NULL) && (argv[i] != NULL); i++) {
		bytes += sizeof(char *) + strlen(argv[i]) + 1;
	}
	for (int i = 0; (envp != NULL) && (envp[i] != NULL); i++) {
		bytes += sizeof(char *) + strlen(envp[i]) + 1;
	}
	if (bytes > CONFIG_SYS_PROCESS_ARG_BYTES) {
		errno = E2BIG;
		return -1;
	}

	return 0;
}

int execve(const char *path, char *const argv[], char *const envp[])
{
	const struct posix_spawn_image *img;
	k_thread_entry_t entry = NULL;
	int ret;

	if (path == NULL) {
		errno = ENOENT;
		return -1;
	}
	if (exec_args_check(argv, envp) != 0) {
		return -1;
	}

	img = posix_spawn_image_lookup(path);
	if ((img != NULL) && (img->entry != NULL)) {
		entry = img->entry;
	} else if (IS_ENABLED(CONFIG_POSIX_EXEC_LLEXT)) {
		/* not a prelinked image: the kernel loads it as the pending image */
		ret = sys_exec_load(k_current_get(), path);
		if (ret < 0) {
			switch (ret) {
			case -ENOEXEC:
				errno = ENOEXEC;
				break;
			case -ENOMEM:
				errno = ENOMEM;
				break;
			case -ENAMETOOLONG:
				errno = ENAMETOOLONG;
				break;
			default:
				errno = ENOENT;
				break;
			}
			return -1;
		}
	} else {
		errno = ENOENT;
		return -1;
	}

	/*
	 * Point of no return: every other member thread is aborted and signal
	 * dispositions revert to default (POSIX); the kernel closes the
	 * FD_CLOEXEC descriptors, swaps the image and restarts the leader.
	 */
	(void)k_process_prune();
#ifdef CONFIG_SIGNAL
	exec_reset_signals();
#endif /* CONFIG_SIGNAL */

	ret = sys_exec_start(entry, argv, envp);
	switch (ret) {
	case -E2BIG:
		errno = E2BIG;
		break;
	case -ENOENT:
		errno = ENOENT;
		break;
	case -ENOEXEC:
		errno = ENOEXEC;
		break;
	case -EINVAL:
		errno = EINVAL;
		break;
	default:
		errno = ENOMEM;
		break;
	}

	return -1;
}
