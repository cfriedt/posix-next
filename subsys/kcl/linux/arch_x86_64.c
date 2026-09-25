/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* x86_64: the thread-local storage base lives in the FS segment base */

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>

#include "kcl_linux.h"

#define ARCH_SET_GS 0x1001
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003
#define ARCH_GET_GS 0x1004

ZKCL_LINUX_IMPL(arch_prctl)(int option, unsigned long arg2)
{
	switch (option) {
	case ARCH_SET_FS:
		arch_user_thread_pointer_set(_current, arg2);
		return 0;
	case ARCH_GET_FS: {
		unsigned long base = arch_user_thread_pointer_get(_current);

		if (zkcl_linux_user_ok((void *)arg2, sizeof(base), true) != 0) {
			return -EFAULT;
		}
		memcpy((void *)arg2, &base, sizeof(base));
		return 0;
	}
	case ARCH_SET_GS:
	case ARCH_GET_GS:
	default:
		return -EINVAL;
	}
}
