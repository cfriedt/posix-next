/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/sys/thread.h>

int pthread_mutex_destroy(pthread_mutex_t *mu)
{
	return -sys_mutex_destroy(to_k_mutex(mu));
}
