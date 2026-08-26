/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

int setpgid(pid_t pid, pid_t pgid)
{
	int ret;
	k_pid_t proc = NULL;
	k_pgrp_t grp = NULL;

	if ((pid < 0) || (pgid < 0)) {
		errno = EINVAL;
		return -1;
	}

	if (pid != 0) {
		proc = sys_process_find((int)pid);
		if (proc == NULL) {
			errno = ESRCH;
			return -1;
		}
	}

	if (pgid != 0) {
		grp = sys_pgrp_find((int)pgid);
		if (grp == NULL) {
			/*
			 * POSIX: pgid may equal the target's pid to create a new
			 * group led by it; any other non-existent group is EPERM.
			 */
			pid_t target = (pid != 0) ? pid : getpid();

			if (pgid != target) {
				errno = EPERM;
				return -1;
			}
			/* grp stays NULL: sys_setpgid() creates the group */
		}
	}

	ret = sys_setpgid(proc, grp);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}
