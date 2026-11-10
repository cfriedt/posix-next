/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/* what XZ Embedded's xz_private.h asks of its host, for Zephyr */

#ifndef ZEPHYR_LIB_XZ_XZ_CONFIG_H_
#define ZEPHYR_LIB_XZ_XZ_CONFIG_H_

#include <stdbool.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#include "xz.h"

#define kmalloc(size, flags) k_malloc(size)
#define kfree(ptr) k_free(ptr)
#define vmalloc(size) k_malloc(size)
#define vfree(ptr) k_free(ptr)

#define memeq(a, b, size) (memcmp(a, b, size) == 0)
#define memzero(buf, size) memset(buf, 0, size)

#ifndef min
#define min(x, y) MIN(x, y)
#endif
#define min_t(type, x, y) MIN(x, y)

#ifndef fallthrough
#define fallthrough __fallthrough
#endif

#define get_unaligned_le32(p) sys_get_le32(p)
#define get_unaligned_be32(p) sys_get_be32(p)
#define put_unaligned_le32(v, p) sys_put_le32(v, p)
#define put_unaligned_be32(v, p) sys_put_be32(v, p)
#define get_le32(p) sys_get_le32((const uint8_t *)(p))

#endif /* ZEPHYR_LIB_XZ_XZ_CONFIG_H_ */
