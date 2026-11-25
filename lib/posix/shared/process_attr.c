/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <stdlib.h>

#include <zephyr/sys/libc-hooks.h>
#include <zephyr/sys/process_state.h>

static Z_LIBC_DATA struct posix_process_attr boot_attr;

struct posix_process_attr *z_posix_boot_process_attr(void)
{
	return &boot_attr;
}

struct posix_process_attr *z_posix_process_attr(void)
{
	struct process_state *lp = process_state_get();
#if PROCESS_STATE_SUPPORTED
	struct posix_process_attr *pa;
#endif

	if (lp == NULL) {
		return &boot_attr;
	}
#if !PROCESS_STATE_SUPPORTED
	return NULL;
#else
	process_state_lock(lp);
	pa = lp->slot[PROCESS_STATE_SLOT_POSIX];
	if (pa == NULL) {
		pa = calloc(1, sizeof(*pa));
		lp->slot[PROCESS_STATE_SLOT_POSIX] = pa;
	}
	process_state_unlock(lp);

	return pa;
#endif /* PROCESS_STATE_SUPPORTED */
}
