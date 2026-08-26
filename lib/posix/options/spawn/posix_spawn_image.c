/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "spawn_internal.h"

#include <stddef.h>

/* the default registry resolves nothing; applications override */
__weak const struct posix_spawn_image *posix_spawn_image_lookup(const char *path)
{
	ARG_UNUSED(path);

	return NULL;
}
