/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>

int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if ((a == NULL) || !a->initialized ||
	    ((pshared != PTHREAD_PROCESS_PRIVATE) && (pshared != PTHREAD_PROCESS_SHARED))) {
		return EINVAL;
	}

	a->pshared = (pshared == PTHREAD_PROCESS_SHARED);

	return 0;
}
