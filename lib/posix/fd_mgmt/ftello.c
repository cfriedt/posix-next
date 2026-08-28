/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <zephyr/sys/util.h>

off_t ftello(FILE *stream)
{
	if (stream == NULL) {
		errno = EBADF;
		return (off_t)-1;
	}

	return lseek(fileno(stream), 0, SEEK_CUR);
}
#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FTELLO
FUNC_ALIAS(ftello, _ftello, off_t);
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FTELLO */

#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FTELLO_R
struct _reent;
off_t _ftello_r(struct _reent *r, FILE *stream)
{
	ARG_UNUSED(r);

	return ftello(stream);
}
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FTELLO_R */
