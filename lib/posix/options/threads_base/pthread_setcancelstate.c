/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_setcancelstate(int state, int *oldstate)
{
	bool enabled;

	if (oldstate != NULL) {
		enabled = k_thread_cancel_getstate();
		*oldstate = enabled ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE;
	}

	switch (state) {
	case PTHREAD_CANCEL_ENABLE:
		enabled = true;
		break;
	case PTHREAD_CANCEL_DISABLE:
		enabled = false;
		break;
	default:
		return EINVAL;
	}

	k_thread_cancel_setstate(enabled);

	return 0;
}
