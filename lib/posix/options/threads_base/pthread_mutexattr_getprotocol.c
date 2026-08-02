/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
	if ((attr == NULL) || (protocol == NULL)) {
		return EINVAL;
	}

	*protocol = PTHREAD_PRIO_NONE;
	return 0;
}
#endif
