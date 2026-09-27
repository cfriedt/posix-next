/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

pid_t getsid(pid_t pid)
{
	k_session_t session;
	k_pid_t proc = NULL;

	if (pid != 0) {
		proc = sys_process_find((int)pid);
		if (proc == NULL) {
			errno = ESRCH;
			return -1;
		}
	}

	session = k_getsid(proc);
	if (session == NULL) {
		errno = ESRCH;
		return -1;
	}

	return (pid_t)sys_session_id(session);
}
