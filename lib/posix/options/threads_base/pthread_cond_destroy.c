/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/sys/thread.h>

int pthread_cond_destroy(pthread_cond_t *cvar)
{
	return -sys_condvar_destroy(to_k_condvar(cvar));
}
