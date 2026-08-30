/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Stand-ins for symbols busybox links against that Zephyr does not provide.
 * They sit on code paths the enabled applets never take at runtime; each
 * fails cleanly if ever reached.
 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <zephyr/llext/symbol.h>

char *realpath(const char *path, char *resolved)
{
	/* no dot-segment or symlink canonicalization: absolute paths pass through */
	if ((path == NULL) || (path[0] != '/')) {
		errno = (path == NULL) ? EINVAL : ENOENT;
		return NULL;
	}

	if (resolved == NULL) {
		return strdup(path);
	}

	(void)strcpy(resolved, path);

	return resolved;
}
EXPORT_SYMBOL(realpath);


/* busybox's disabled ulimit builtin still links these from a live object */
struct rlimit;

int getrlimit(int resource, struct rlimit *rlp)
{
	ARG_UNUSED(resource);
	ARG_UNUSED(rlp);
	errno = ENOSYS;
	return -1;
}
EXPORT_SYMBOL(getrlimit);

int setrlimit(int resource, const struct rlimit *rlp)
{
	ARG_UNUSED(resource);
	ARG_UNUSED(rlp);
	errno = ENOSYS;
	return -1;
}
EXPORT_SYMBOL(setrlimit);

int chroot(const char *path)
{
	ARG_UNUSED(path);
	errno = ENOSYS;
	return -1;
}
EXPORT_SYMBOL(chroot);

int mallopt(int param, int value)
{
	ARG_UNUSED(param);
	ARG_UNUSED(value);
	return 0;
}
EXPORT_SYMBOL(mallopt);

char *mktemp(char *tmpl)
{
	if (tmpl != NULL) {
		tmpl[0] = '\0';
	}
	errno = ENOSYS;
	return tmpl;
}
EXPORT_SYMBOL(mktemp);

int settimeofday(const struct timeval *tv, const void *tz)
{
	ARG_UNUSED(tv);
	ARG_UNUSED(tz);
	errno = ENOSYS;
	return -1;
}
EXPORT_SYMBOL(settimeofday);



char *dirname(char *path)
{
	char *slash;

	if ((path == NULL) || (*path == '\0')) {
		return ".";
	}

	slash = strrchr(path, '/');
	if (slash == NULL) {
		return ".";
	}
	if (slash == path) {
		return "/";
	}
	*slash = '\0';

	return path;
}
EXPORT_SYMBOL(dirname);

#ifndef CONFIG_POSIX_NETWORKING
int socket(int domain, int type, int protocol)
{
	ARG_UNUSED(domain);
	ARG_UNUSED(type);
	ARG_UNUSED(protocol);
	errno = EAFNOSUPPORT;
	return -1;
}
EXPORT_SYMBOL(socket);

int bind(int sock, const struct sockaddr *addr, socklen_t addrlen)
{
	ARG_UNUSED(sock);
	ARG_UNUSED(addr);
	ARG_UNUSED(addrlen);
	errno = EBADF;
	return -1;
}
EXPORT_SYMBOL(bind);

int listen(int sock, int backlog)
{
	ARG_UNUSED(sock);
	ARG_UNUSED(backlog);
	errno = EBADF;
	return -1;
}
EXPORT_SYMBOL(listen);

ssize_t sendto(int sock, const void *buf, size_t len, int flags,
	       const struct sockaddr *dest_addr, socklen_t addrlen)
{
	ARG_UNUSED(sock);
	ARG_UNUSED(buf);
	ARG_UNUSED(len);
	ARG_UNUSED(flags);
	ARG_UNUSED(dest_addr);
	ARG_UNUSED(addrlen);
	errno = EBADF;
	return -1;
}
EXPORT_SYMBOL(sendto);
#endif /* CONFIG_POSIX_NETWORKING */
