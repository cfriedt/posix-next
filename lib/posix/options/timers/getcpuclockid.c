/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/sys/util.h>

#if defined(_POSIX_CPUTIME)
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
#endif

#if defined(_POSIX_THREAD_CPUTIME)
int pthread_getcpuclockid(pthread_t thread_id, clockid_t *clock_id)
{
	ARG_UNUSED(thread_id);

#ifndef CONFIG_NATIVE_LIBC
	if (clock_id == NULL) {
		return EINVAL;
	}
#endif

	*clock_id = CLOCK_THREAD_CPUTIME_ID;

	return 0;
}
#endif
