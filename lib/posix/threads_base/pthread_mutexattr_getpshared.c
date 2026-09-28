/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>

int pthread_mutexattr_getpshared(const pthread_mutexattr_t *ZRESTRICT attr, int *ZRESTRICT pshared)
{
	const struct pthread_mutexattr *const a = (const struct pthread_mutexattr *)attr;

	if ((a == NULL) || (pshared == NULL) || !a->initialized) {
		return EINVAL;
	}

	*pshared = a->pshared ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;

	return 0;
}
