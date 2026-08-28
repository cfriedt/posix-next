/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mqueue_internal.h"

#include <stdarg.h>
#include <sys/stat.h>

mqd_t mq_open(const char *name, int oflags, ...)
{
	int ret;
	va_list va;
	struct k_msgq_attrs kattrs;
	const struct mq_attr *attrs = NULL;
	mode_t mode = 0;

	if ((oflags & O_CREAT) != 0) {
		va_start(va, oflags);
		BUILD_ASSERT(sizeof(mode_t) <= sizeof(int));
		mode = va_arg(va, unsigned int);
		attrs = va_arg(va, const struct mq_attr *);
		va_end(va);

		if ((attrs == NULL) || (attrs->mq_msgsize <= 0) || (attrs->mq_maxmsg <= 0)) {
			errno = EINVAL;
			return (mqd_t)-1;
		}

		kattrs = (struct k_msgq_attrs){
			.msg_size = (size_t)attrs->mq_msgsize,
			.max_msgs = (uint32_t)attrs->mq_maxmsg,
		};
	}

	ret = sys_msgq_open(name, to_oflags(oflags), (uint32_t)mode,
			    (attrs == NULL) ? NULL : &kattrs);
	if (ret < 0) {
		errno = -ret;
		return (mqd_t)-1;
	}

	return (mqd_t)ret;
}
