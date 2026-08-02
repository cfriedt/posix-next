/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_mutex_unlock(pthread_mutex_t *mu)
{
	return -k_mutex_unlock(to_k_mutex(mu));
}
