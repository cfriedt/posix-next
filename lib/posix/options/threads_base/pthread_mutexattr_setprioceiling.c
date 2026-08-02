/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>

#if defined(_POSIX_THREAD_PRIO_PROTECT)
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
	ARG_UNUSED(attr);
	ARG_UNUSED(prioceiling);

	return ENOSYS;
}
#endif
