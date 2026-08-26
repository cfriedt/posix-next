/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_LIB_POSIX_POSIX_IMAGE_H_
#define ZEPHYR_LIB_POSIX_POSIX_IMAGE_H_

#include <zephyr/kernel.h>

/* An executable image a spawn or exec path may name before exec loading (M3) */
struct posix_spawn_image {
	k_thread_entry_t entry;
};

/*
 * Resolve a path to a prelinked image. The default resolves nothing
 * (ENOENT); applications and test suites override with their own registry.
 */
const struct posix_spawn_image *posix_spawn_image_lookup(const char *path);

#endif /* ZEPHYR_LIB_POSIX_POSIX_IMAGE_H_ */
