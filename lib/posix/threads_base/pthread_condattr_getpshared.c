/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>

int pthread_condattr_getpshared(const pthread_condattr_t *ZRESTRICT att, int *ZRESTRICT pshared)
{
	const struct posix_condattr *const attr = (const struct posix_condattr *)att;

	if ((attr == NULL) || (pshared == NULL) || !attr->initialized) {
		return EINVAL;
	}

	*pshared = attr->pshared ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;

	return 0;
}
