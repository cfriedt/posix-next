/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sched.h>

#include <zephyr/kernel.h>

int sched_yield(void)
{
	k_yield();
	return 0;
}
