/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>

void pthread_testcancel(void)
{
	if (k_thread_cancel_may_be_pending()) {
		k_thread_testcancel();
	}
}
