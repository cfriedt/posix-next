/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <zephyr/sys/util.h>

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
