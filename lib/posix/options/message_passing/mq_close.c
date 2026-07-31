/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mqueue_internal.h"

int mq_close(mqd_t mqdes)
{
	return zvfs_close((int)mqdes);
}
