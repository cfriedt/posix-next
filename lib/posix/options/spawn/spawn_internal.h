/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_LIB_POSIX_SPAWN_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_SPAWN_INTERNAL_H_

#include <spawn.h>
#include <sys/types.h>

#include <zephyr/kernel.h>

enum posix_spawn_file_action_type {
	POSIX_SPAWN_FILE_ACTION_OPEN,
	POSIX_SPAWN_FILE_ACTION_CLOSE,
	POSIX_SPAWN_FILE_ACTION_DUP2,
};

struct posix_spawn_file_action {
	enum posix_spawn_file_action_type type;
	int fildes;
	int newfildes;
	char *path;
	int oflag;
	mode_t mode;
};

/* An executable image a spawn path may name before exec exists (M3) */
struct posix_spawn_image {
	k_thread_entry_t entry;
};

/*
 * Resolve a spawn path to a prelinked image. The default resolves nothing
 * (ENOENT); applications and test suites override with their own registry.
 */
const struct posix_spawn_image *posix_spawn_image_lookup(const char *path);

#endif /* ZEPHYR_LIB_POSIX_SPAWN_INTERNAL_H_ */
