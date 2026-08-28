/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <setjmp.h>
#include <signal.h>

#include <zephyr/kernel.h>

int __sigjmp_save(struct __sigjmp_buf *env, int savemask)
{
	env->__savemask = savemask;

	if (savemask != 0) {
		struct k_sig_set okset;
		sigset_t oset_buf;

		(void)k_sig_mask(K_SIG_SETMASK, NULL, &okset);
		env->__mask = *z_sig_set_to_posix(&okset, &oset_buf);
	}

	return 0;
}
