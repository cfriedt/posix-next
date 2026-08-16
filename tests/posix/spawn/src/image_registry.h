/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef TESTS_POSIX_SPAWN_IMAGE_REGISTRY_H_
#define TESTS_POSIX_SPAWN_IMAGE_REGISTRY_H_

#include <zephyr/kernel.h>

#include "posix_image.h"

struct image_registry_entry {
	const char *path;
	struct posix_spawn_image image;
};

#define IMAGE_REGISTRY_ENTRY_DEFINE(name, _path, _entry)                                           \
	static STRUCT_SECTION_ITERABLE(image_registry_entry, name) = {                       \
		.path = (_path),                                                                   \
		.image = {.entry = (_entry)},                                                      \
	}

#endif /* TESTS_POSIX_SPAWN_IMAGE_REGISTRY_H_ */
