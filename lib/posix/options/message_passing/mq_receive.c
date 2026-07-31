/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mqueue_internal.h"

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio)
{
	return mq_receive_common(mqdes, msg_ptr, msg_len, msg_prio, K_FOREVER, false);
}
