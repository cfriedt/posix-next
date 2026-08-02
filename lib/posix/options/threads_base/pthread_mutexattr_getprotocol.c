/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
	const struct pthread_mutexattr *const a = (const struct pthread_mutexattr *)attr;

	if ((a == NULL) || (protocol == NULL) || !a->initialized) {
		return EINVAL;
	}

	*protocol = a->protocol;
	return 0;
}
#endif
