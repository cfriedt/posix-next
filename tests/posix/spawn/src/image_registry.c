/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * A reusable testsuite-local registry backing posix_spawn_image_lookup():
 * spawn paths name prelinked images until exec exists.
 */

#include <string.h>

#include "image_registry.h"
#include "posix_image.h"

const struct posix_spawn_image *posix_spawn_image_lookup(const char *path)
{
	STRUCT_SECTION_FOREACH(image_registry_entry, entry) {
		if (strcmp(entry->path, path) == 0) {
			return &entry->image;
		}
	}

	return NULL;
}
