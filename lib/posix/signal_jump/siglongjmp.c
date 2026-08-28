/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <setjmp.h>
#include <signal.h>

#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>

FUNC_NORETURN void siglongjmp(sigjmp_buf env, int val)
{
	struct k_sig_set kset_buf;

	/* a jump out of a signal handler ends that handler, with or without a saved mask */
	(void)k_sig_abandon((env->__savemask != 0) ? z_sig_set_from_posix(&env->__mask, &kset_buf)
						   : NULL);

	longjmp(env->__env, val);
	CODE_UNREACHABLE;
}
