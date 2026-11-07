/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fcntl.h>
#include <unistd.h>

#include "posix_access.h"

int faccessat(int fd, const char *path, int amode, int flag)
{
	/* the real and effective identities are the same: AT_EACCESS changes nothing */
	return posix_access_check(fd, path, amode, flag);
}
