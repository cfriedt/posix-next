/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_process_internal.h"

#include <errno.h>
#include <sys/wait.h>

#include <zephyr/kernel.h>

pid_t waitpid(pid_t pid, int *stat_loc, int options)
{
	int ret;
	int kws = 0;
	pid_t num = -1;

	if ((options & ~(WNOHANG | WUNTRACED | K_PROCESS_WCONTINUED)) != 0) {
		errno = EINVAL;
		return -1;
	}

	if (pid == -1) {
		ret = posix_wait_common(NULL, NULL, false, &num, &kws, options);
	} else if (pid == 0) {
		ret = posix_wait_common(NULL, NULL, true, &num, &kws, options);
	} else if (pid < -1) {
		k_pgrp_t grp = sys_pgrp_find((int)-pid);

		if (grp == NULL) {
			errno = ECHILD;
			return -1;
		}
		ret = posix_wait_common(NULL, grp, true, &num, &kws, options);
	} else {
		k_pid_t child = sys_process_find((int)pid);

		if (child == NULL) {
			errno = ECHILD;
			return -1;
		}
		ret = posix_wait_common(child, NULL, false, &num, &kws, options);
	}

	if (ret == -EAGAIN) {
		/* WNOHANG with no state change */
		return 0;
	}
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	if (stat_loc != NULL) {
		*stat_loc = wstatus_to_posix(kws);
	}

	return num;
}
