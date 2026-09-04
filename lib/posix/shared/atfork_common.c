/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>

#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>

int z_posix_atfork_register(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
#if POSIX_THREAD_ATFORK_MAX == 0
	ARG_UNUSED(prepare);
	ARG_UNUSED(parent);
	ARG_UNUSED(child);

	return ENOMEM;
#else
	struct posix_process_attr *pa = z_posix_process_attr();
	atomic_val_t slot;

	if (pa == NULL) {
		return ENOMEM;
	}

	do {
		slot = atomic_get(&pa->atfork_count);
		if (slot >= POSIX_THREAD_ATFORK_MAX) {
			return ENOMEM;
		}
	} while (!atomic_cas(&pa->atfork_count, slot, slot + 1));

	pa->atfork[slot].prepare = prepare;
	pa->atfork[slot].parent = parent;
	pa->atfork[slot].child = child;

	return 0;
#endif
}

void z_posix_atfork_run(enum posix_atfork_stage stage)
{
	struct posix_process_attr *pa = z_posix_process_attr();
	struct posix_atfork *atfork_handlers;
	int n;

	if (pa == NULL) {
		return;
	}
	atfork_handlers = pa->atfork;
	n = (int)atomic_get(&pa->atfork_count);

	switch (stage) {
	case POSIX_ATFORK_PREPARE:
		/* reverse registration order, per POSIX */
		for (int i = n - 1; i >= 0; i--) {
			if (atfork_handlers[i].prepare != NULL) {
				atfork_handlers[i].prepare();
			}
		}
		break;
	case POSIX_ATFORK_PARENT:
		for (int i = 0; i < n; i++) {
			if (atfork_handlers[i].parent != NULL) {
				atfork_handlers[i].parent();
			}
		}
		break;
	case POSIX_ATFORK_CHILD:
		for (int i = 0; i < n; i++) {
			if (atfork_handlers[i].child != NULL) {
				atfork_handlers[i].child();
			}
		}
		break;
	default:
		break;
	}
}
