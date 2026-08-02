/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include <zephyr/kernel.h>

int pthread_kill(pthread_t thread, int sig)
{
	int ksigno;
	int ret;

	if (sig == 0) {
		ksigno = 0;
	} else {
		ksigno = z_sig_from_posix(sig);
		if (ksigno <= 0) {
			return EINVAL;
		}
	}

	ret = k_sig_queue(to_k_thread(&thread), ksigno, (union k_sig_val){0});

	return -ret;
}
