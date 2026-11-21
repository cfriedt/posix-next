/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rlimit_internal.h"

#include <errno.h>
#include <stddef.h>
#include <sys/resource.h>

#include <zephyr/kernel.h>

int z_posix_rlimit(int resource, rlim_t *limit)
{
	switch (resource) {
	case RLIMIT_CORE:
		/* no core file is ever written */
		*limit = 0;
		break;
	case RLIMIT_NOFILE:
		/* the descriptor table: CONFIG_ZVFS_OPEN_MAX and what the libraries add to it */
		*limit = ZVFS_OPEN_SIZE;
		break;
	case RLIMIT_NPROC:
		/* the process records, static and heap-provided */
		*limit = ((SYS_PROCESS_MIN + CONFIG_SYS_PROCESS_MAX) > 0)
				 ? (SYS_PROCESS_MIN + CONFIG_SYS_PROCESS_MAX)
				 : RLIM_INFINITY;
		break;
	case RLIMIT_STACK:
#ifdef CONFIG_DYNAMIC_THREAD_STACK_SIZE
		/* the stack a new thread or process gets */
		*limit = CONFIG_DYNAMIC_THREAD_STACK_SIZE;
#else
		*limit = RLIM_INFINITY;
#endif
		break;
	case RLIMIT_CPU:
	case RLIMIT_FSIZE:
	case RLIMIT_DATA:
	case RLIMIT_RSS:
	case RLIMIT_MEMLOCK:
	case RLIMIT_AS:
		*limit = RLIM_INFINITY;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int getrlimit(int resource, struct rlimit *rlp)
{
	rlim_t limit;
	int ret;

	if (rlp == NULL) {
		errno = EINVAL;
		return -1;
	}
	ret = z_posix_rlimit(resource, &limit);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	/* the hard limit is the soft limit: nothing raises either */
	rlp->rlim_cur = limit;
	rlp->rlim_max = limit;

	return 0;
}
