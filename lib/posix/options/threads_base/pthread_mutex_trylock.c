/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <errno.h>
#include <pthread.h>

int pthread_mutex_trylock(pthread_mutex_t *m)
{
	int ret;

	ret = pthread_mutex_lock_common(m, K_NO_WAIT);

	return (ret == EAGAIN) ? EBUSY : ret;
}
