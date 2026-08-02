/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>

#if defined(_POSIX_THREAD_PRIO_PROTECT)
int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling)
{
	ARG_UNUSED(mutex);
	ARG_UNUSED(prioceiling);

	return ENOSYS;
}
#endif
