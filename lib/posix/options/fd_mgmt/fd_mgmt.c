/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/zvfs.h>

int dup(int fildes)
{
	return zvfs_dup(fildes, 0);
}

int dup2(int fildes, int fildes2)
{
	return zvfs_dup2(fildes, fildes2);
}

int fcntl(int fd, int cmd, ...)
{
	int ret;
	va_list args;

	va_start(args, cmd);
	ret = zvfs_fcntl(fd, cmd, args);
	va_end(args);

	return ret;
}
#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FCNTL
FUNC_ALIAS(fcntl, _fcntl, int);
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FCNTL */

int ftruncate(int fd, off_t length)
{
	return zvfs_ftruncate(fd, length);
}
#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FTRUNCATE
FUNC_ALIAS(ftruncate, _ftruncate, int);
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FTRUNCATE */

off_t lseek(int fd, off_t offset, int whence)
{
	return zvfs_lseek(fd, offset, whence);
}
#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_LSEEK
FUNC_ALIAS(lseek, _lseek, off_t);
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_LSEEK */

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

#if defined(CONFIG_POSIX_FD_MGMT_ALIAS_FSEEKO_R) || defined(CONFIG_POSIX_FD_MGMT_ALIAS_FTELLO_R)
#include <reent.h>
#endif

#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FSEEKO_R
int _fseeko_r(struct _reent *r, FILE *stream, _off_t offset, int whence)
{
	ARG_UNUSED(r);

	return fseeko(stream, offset, whence);
}
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FSEEKO_R */

#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FTELLO_R
_off_t _ftello_r(struct _reent *r, FILE *stream)
{
	ARG_UNUSED(r);

	return ftello(stream);
}
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FTELLO_R */
