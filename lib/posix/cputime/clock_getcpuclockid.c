/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <time.h>
#include <unistd.h>

int clock_getcpuclockid(pid_t pid, clockid_t *clock_id)
{
#ifdef CONFIG_POSIX_MULTI_PROCESS
	if (pid != 0 && pid != getpid()) {
		return EPERM;
	}
#else
	if (pid != 0) {
		return EPERM;
	}
#endif

	*clock_id = CLOCK_PROCESS_CPUTIME_ID;

	return 0;
}
