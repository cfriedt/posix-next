/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <zephyr/sys/util.h>

int fseeko(FILE *stream, off_t offset, int whence)
{
	off_t pos;

	if (stream == NULL) {
		errno = EBADF;
		return -1;
	}

	pos = lseek(fileno(stream), offset, whence);

	return (pos == (off_t)-1) ? -1 : 0;
}
#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FSEEKO
FUNC_ALIAS(fseeko, _fseeko, int);
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FSEEKO */

#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FSEEKO_R
struct _reent;
int _fseeko_r(struct _reent *r, FILE *stream, _off_t offset, int whence)
{
	ARG_UNUSED(r);

	return fseeko(stream, offset, whence);
}
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FSEEKO_R */
