/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mqueue_internal.h"

int mq_unlink(const char *name)
{
	int ret = sys_msgq_unlink(name);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}
