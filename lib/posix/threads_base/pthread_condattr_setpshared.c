/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>

int pthread_condattr_setpshared(pthread_condattr_t *att, int pshared)
{
	struct posix_condattr *const attr = (struct posix_condattr *)att;

	if ((attr == NULL) || !attr->initialized ||
	    ((pshared != PTHREAD_PROCESS_PRIVATE) && (pshared != PTHREAD_PROCESS_SHARED))) {
		return EINVAL;
	}

	attr->pshared = (pshared == PTHREAD_PROCESS_SHARED);

	return 0;
}
