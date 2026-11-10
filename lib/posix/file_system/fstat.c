/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_stat.h"

#include <errno.h>
#include <sys/stat.h>

#include <zephyr/sys/zvfs.h>
#include <zephyr/toolchain.h>

int fstat(int fildes, struct stat *buf)
{
	struct zvfs_stat zs;
	int rc;

	if (buf == NULL) {
		errno = EFAULT;
		return -1;
	}

	rc = zvfs_fstat(fildes, &zs);
	if (rc < 0) {
		return -1;
	}

	posix_stat_from_zvfs(buf, &zs);
	return 0;
}
#ifdef CONFIG_POSIX_FILE_SYSTEM_ALIAS_FSTAT
FUNC_ALIAS(fstat, _fstat, int);
#endif
