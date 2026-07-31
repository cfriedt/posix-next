/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mqueue_internal.h"

#include <time.h>

#include <zephyr/sys/timeutil.h>

ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio,
			const struct timespec *abstime)
{
	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		errno = EINVAL;
		return -1;
	}

	return mq_receive_common(mqdes, msg_ptr, msg_len, msg_prio,
				 timespec_abs_to_timeout(SYS_CLOCK_REALTIME, abstime), true);
}
