/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_process_internal.h"

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include <zephyr/kernel.h>

int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
	int ret;
	int kws = 0;
	pid_t num = -1;

	if ((options & ~(WEXITED | WNOHANG | WNOWAIT | WSTOPPED | K_PROCESS_WCONTINUED)) != 0) {
		errno = EINVAL;
		return -1;
	}

	switch (idtype) {
	case P_ALL:
		ret = posix_wait_common(NULL, NULL, false, &num, &kws, options);
		break;
	case P_PID: {
		k_pid_t child = sys_process_find((int)id);

		if (child == NULL) {
			errno = ECHILD;
			return -1;
		}
		ret = posix_wait_common(child, NULL, false, &num, &kws, options);
		break;
	}
	case P_PGID: {
		k_pgrp_t grp = NULL;

		if (id != 0) {
			grp = sys_pgrp_find((int)id);
			if (grp == NULL) {
				errno = ECHILD;
				return -1;
			}
		}
		ret = posix_wait_common(NULL, grp, true, &num, &kws, options);
		break;
	}
	default:
		errno = EINVAL;
		return -1;
	}

	if (ret == -EAGAIN) {
		/* WNOHANG with no state change: infop->si_pid is left 0 */
		if (infop != NULL) {
			*infop = (siginfo_t){0};
		}
		return 0;
	}
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	if (infop != NULL) {
		*infop = (siginfo_t){
			.si_signo = SIGCHLD,
			.si_pid = num,
			.si_status = K_WIFEXITED(kws) ? K_WEXITSTATUS(kws)
						      : z_sig_to_posix(K_WTERMSIG(kws)),
			.si_code = K_WIFEXITED(kws) ? CLD_EXITED : CLD_KILLED,
		};
	}

	return 0;
}
