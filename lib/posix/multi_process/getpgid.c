/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

pid_t getpgid(pid_t pid)
{
	k_pgrp_t grp;
	k_pid_t proc = NULL;

	if (pid != 0) {
		proc = sys_process_find((int)pid);
		if (proc == NULL) {
			errno = ESRCH;
			return -1;
		}
	}

	grp = k_getpgid(proc);
	if (grp == NULL) {
		errno = ESRCH;
		return -1;
	}

	return (pid_t)sys_pgrp_id(grp);
}
