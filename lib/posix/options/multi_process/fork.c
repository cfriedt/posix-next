/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

#include "posix_internal.h"

pid_t fork(void)
{
	int ret;
	k_pid_t child = NULL;

	z_posix_atfork_run(POSIX_ATFORK_PREPARE);

	/*
	 * The kernel resumes the child from this call's captured syscall
	 * frame: both sides return here with 0, and the child is told apart
	 * by its memory copy - it was taken before the parent's handle was
	 * written, so the child reads its pre-fork NULL.
	 */
	ret = sys_fork(&child);
	if (ret < 0) {
		z_posix_atfork_run(POSIX_ATFORK_PARENT);
		errno = -ret;
		return -1;
	}

	if (child == NULL) {
		z_posix_atfork_run(POSIX_ATFORK_CHILD);
		return 0;
	}

	z_posix_atfork_run(POSIX_ATFORK_PARENT);

	return (pid_t)sys_process_id(child);
}
