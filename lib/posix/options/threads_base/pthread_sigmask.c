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
	int k_how;

	switch (how) {
	case SIG_BLOCK:
		k_how = K_SIG_BLOCK;
		break;
	case SIG_SETMASK:
		k_how = K_SIG_SETMASK;
		break;
	case SIG_UNBLOCK:
		k_how = K_SIG_UNBLOCK;
		break;
	default:
		return EINVAL;
	}

	struct k_sig_set kset_buf, okset;
	const struct k_sig_set *kset = NULL;
	sigset_t oset_buf;

	if (set != NULL) {
		kset = z_sig_set_from_posix(set, &kset_buf);
	}

	int ret = k_sig_mask(k_how, kset, oset == NULL ? NULL : &okset);

	if (ret < 0) {
		return -ret;
	}

	if (oset != NULL) {
		*oset = *z_sig_set_to_posix(&okset, &oset_buf);
	}

	return 0;
}
