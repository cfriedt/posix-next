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

int pthread_sigmask(int how, const sigset_t *ZRESTRICT set, sigset_t *ZRESTRICT oset)
{
#ifdef CONFIG_SIGNAL
	return z_sig_posix_mask(how, set, oset);
#else
	ARG_UNUSED(how);
	ARG_UNUSED(set);
	ARG_UNUSED(oset);

	return ENOTSUP;
#endif /* CONFIG_SIGNAL */
}
