/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <zephyr/kernel.h>

#include "posix_image.h"

int execve(const char *path, char *const argv[], char *const envp[])
{
	const struct posix_spawn_image *img;

	if (path == NULL) {
		errno = ENOENT;
		return -1;
	}

	img = posix_spawn_image_lookup(path);
	if ((img == NULL) || (img->entry == NULL)) {
		errno = ENOENT;
		return -1;
	}

	/*
	 * The process image is replaced in place: every other member thread
	 * is aborted and the calling thread continues as the new image, which
	 * preserves the process's identity, parent, and group membership.
	 * Deviations before exec loading (M3): the new image runs on the
	 * calling thread's stack, and signal dispositions and FD_CLOEXEC are
	 * not yet reset.
	 */
	(void)k_process_prune();

	img->entry((void *)argv, (void *)envp, NULL);

	/* the image's entry returned: exit as if main() returned 0 */
	exit(0);
}
