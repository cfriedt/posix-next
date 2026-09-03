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

#include <arpa/inet.h>
#include <errno.h>
#include <grp.h>
#include <langinfo.h>
#include <libgen.h>
#include <locale.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/utsname.h>
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
int gethostname(char *name, size_t len)
{
	struct utsname uts;

	if (uname(&uts) < 0) {
		return -1;
	}
	if (strlen(uts.nodename) >= len) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(name, uts.nodename);

	return 0;
}
EXPORT_SYMBOL(gethostname);

int connect(int sock, const struct sockaddr *addr, socklen_t addrlen)
{
	ARG_UNUSED(sock);
	ARG_UNUSED(addr);
	ARG_UNUSED(addrlen);

	errno = ENOSYS;
	return -1;
}
EXPORT_SYMBOL(connect);

int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen)
{
	ARG_UNUSED(sock);
	ARG_UNUSED(level);
	ARG_UNUSED(optname);
	ARG_UNUSED(optval);
	ARG_UNUSED(optlen);

	errno = ENOSYS;
	return -1;
}
EXPORT_SYMBOL(setsockopt);

int shutdown(int sock, int how)
{
	ARG_UNUSED(sock);
	ARG_UNUSED(how);

	errno = ENOSYS;
	return -1;
}
EXPORT_SYMBOL(shutdown);

ssize_t recvfrom(int sock, void *buf, size_t len, int flags, struct sockaddr *addr,
		 socklen_t *addrlen)
{
	ARG_UNUSED(sock);
	ARG_UNUSED(buf);
	ARG_UNUSED(len);
	ARG_UNUSED(flags);
	ARG_UNUSED(addr);
	ARG_UNUSED(addrlen);

	errno = ENOSYS;
	return -1;
}
EXPORT_SYMBOL(recvfrom);

int getaddrinfo(const char *host, const char *service, const struct addrinfo *hints,
		struct addrinfo **res)
{
	ARG_UNUSED(host);
	ARG_UNUSED(service);
	ARG_UNUSED(hints);
	ARG_UNUSED(res);

	return EAI_FAIL;
}
EXPORT_SYMBOL(getaddrinfo);

void freeaddrinfo(struct addrinfo *ai)
{
	ARG_UNUSED(ai);
}
EXPORT_SYMBOL(freeaddrinfo);

const char *gai_strerror(int errcode)
{
	ARG_UNUSED(errcode);

	return "Name resolution unavailable";
}
EXPORT_SYMBOL(gai_strerror);

char *inet_ntop(sa_family_t family, const void *src, char *dst, size_t size)
{
	ARG_UNUSED(family);
	ARG_UNUSED(src);
	ARG_UNUSED(dst);
	ARG_UNUSED(size);

	errno = EAFNOSUPPORT;
	return NULL;
}
EXPORT_SYMBOL(inet_ntop);
#endif /* CONFIG_POSIX_NETWORKING */

/* XSI basename(): the last component, with trailing slashes stripped.
 * The libc header may alias the name; the extension resolves the plain one.
 */
#undef basename
char *basename(char *path)
{
	static char dot[] = {'.', '\0'};
	char *end;
	char *start;

	if (path == NULL || *path == '\0') {
		return dot;
	}

	end = path + strlen(path) - 1;
	while (end > path && *end == '/') {
		*end-- = '\0';
	}

	if (end == path && *end == '/') {
		return path;
	}

	start = strrchr(path, '/');

	return (start == NULL) ? path : start + 1;
}
EXPORT_SYMBOL(basename);

/* one identity, no supplementary groups */
int initgroups(const char *user, gid_t group)
{
	ARG_UNUSED(user);
	ARG_UNUSED(group);

	return 0;
}
EXPORT_SYMBOL(initgroups);

/* provided by the C library; their Option Groups (XSI_C_LANG_SUPPORT,
 * XSI_WIDE_CHAR, MULTI_CONCURRENT_LOCALES, I18N) are not yet claimed by
 * the POSIX layer, so the export lives here
 */
EXPORT_SYMBOL(strptime);
EXPORT_SYMBOL(wcwidth);
EXPORT_SYMBOL(newlocale);
EXPORT_SYMBOL(uselocale);
EXPORT_SYMBOL(nl_langinfo);
EXPORT_SYMBOL(fmemopen);
EXPORT_SYMBOL(random);
EXPORT_SYMBOL(srandom);
