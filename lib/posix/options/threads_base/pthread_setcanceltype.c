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

int pthread_setcanceltype(int type, int *oldtype)
{
	const bool async = true;
	bool n_type;
	bool o_type;

	switch (type) {
	case PTHREAD_CANCEL_ASYNCHRONOUS:
		n_type = async;
		break;
	case PTHREAD_CANCEL_DEFERRED:
		n_type = !async;
		break;
	default:
		return EINVAL;
	}

	k_thread_cancel_qtype(&n_type, &o_type);

	if (oldtype != NULL) {
		*oldtype =
			(o_type == async) ? PTHREAD_CANCEL_ASYNCHRONOUS : PTHREAD_CANCEL_DEFERRED;
	}

	return 0;
}
