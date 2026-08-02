/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/toolchain.h>

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
	ARG_UNUSED(prepare);
	ARG_UNUSED(parent);
	ARG_UNUSED(child);

	/*
	 * Zephyr has no fork(), so registered handlers could never run;
	 * accepting and discarding them is conformant.
	 */
	return 0;
}
