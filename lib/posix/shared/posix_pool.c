/*
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <stddef.h>
#include <stdint.h>

#include <zephyr/sys/elastipool.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

void *posix_get_pool_obj_unlocked(const struct sys_elastipool *pool, uint32_t handle)
{
	uintptr_t ret;

	if (sizeof(ret) == sizeof(handle)) {
		ret = handle;
	} else if (sizeof(ret) == 2 * sizeof(handle)) {
		/* assumption is that the heap lies in the same 32-bit address range */
		ret = (uintptr_t)pool->config->storage;
		ret &= 0xffffffff00000000ULL;
		ret |= handle;
	} else {
		/* unsupported size */
		return NULL;
	}

	if (sys_elastipool_check(pool, (void *)ret) < 0) {
		/* object is uninitialized */
		ret = 0; /* NULL */
	}

	return (void *)ret;
}

void *posix_init_pool_obj_unlocked(const struct sys_elastipool *pool, uint32_t handle,
				   void (*cb)(void *obj))
{
	if (handle != POSIX_OBJ_INITIALIZER) {
		return posix_get_pool_obj_unlocked(pool, handle);
	}

	uintptr_t obj = 0;

	if (sys_elastipool_alloc(pool, (void **)&obj) < 0) {
		/* no objects left to allocate */
		return NULL;
	}

	if ((obj != 0) && (cb != NULL)) {
		cb((void *)obj);
	}

	return (void *)obj;
}
