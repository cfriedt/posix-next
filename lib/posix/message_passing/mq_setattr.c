/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mqueue_internal.h"

int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
	int ret;

	if (mqstat == NULL) {
		errno = EINVAL;
		return -1;
	}

	if ((mqstat->mq_flags & ~(long)O_NONBLOCK) != 0) {
		errno = EINVAL;
		return -1;
	}

	if (omqstat != NULL) {
		ret = mq_getattr(mqdes, omqstat);
		if (ret < 0) {
			return -1;
		}
	}

	ret = sys_msgq_setflags((int)mqdes,
				((mqstat->mq_flags & O_NONBLOCK) != 0) ? ZVFS_O_NONBLOCK : 0);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}
