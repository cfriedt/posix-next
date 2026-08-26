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

#include "posix_image.h"

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


#endif /* ZEPHYR_LIB_POSIX_SPAWN_INTERNAL_H_ */
