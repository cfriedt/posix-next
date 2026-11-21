/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rlimit_internal.h"

#include <errno.h>
#include <stddef.h>
#include <sys/resource.h>

int setrlimit(int resource, const struct rlimit *rlp)
{
	rlim_t limit;
	int ret;

	if (rlp == NULL) {
		errno = EINVAL;
		return -1;
	}
	ret = z_posix_rlimit(resource, &limit);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	if (rlp->rlim_cur > rlp->rlim_max) {
		errno = EINVAL;
		return -1;
	}
	/* the limits are the build's: the current ones are accepted, any change is refused */
	if ((rlp->rlim_cur != limit) || (rlp->rlim_max != limit)) {
		errno = EPERM;
		return -1;
	}

	return 0;
}
