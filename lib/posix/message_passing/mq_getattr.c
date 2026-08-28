/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mqueue_internal.h"

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
	int ret;
	int oflags;
	struct k_msgq_attrs kattrs;

	if (mqstat == NULL) {
		errno = EINVAL;
		return -1;
	}

	ret = sys_msgq_getattr((int)mqdes, &kattrs, &oflags);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	*mqstat = (struct mq_attr){
		.mq_flags = ((oflags & ZVFS_O_NONBLOCK) != 0) ? O_NONBLOCK : 0,
		.mq_maxmsg = (long)kattrs.max_msgs,
		.mq_msgsize = (long)kattrs.msg_size,
		.mq_curmsgs = (long)kattrs.used_msgs,
	};

	return 0;
}
