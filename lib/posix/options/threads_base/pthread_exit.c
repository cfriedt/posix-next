/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>

FUNC_NORETURN
void pthread_exit(void *retval)
{
	k_thread_exit(retval);
	CODE_UNREACHABLE;
}
