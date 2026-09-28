/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "image_registry.h"

#ifdef CONFIG_POSIX_SPAWN
const struct posix_spawn_image *posix_spawn_image_lookup(const char *path)
{
	STRUCT_SECTION_FOREACH(image_registry_entry, entry) {
		if (strcmp(entry->path, path) == 0) {
			return &entry->image;
		}
	}

	return NULL;
}
#endif /* CONFIG_POSIX_SPAWN */
