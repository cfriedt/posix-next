/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/* the host lookup of POSIX.1-2001: <netdb.h> declares it to a program asking for the extensions */
#define _DEFAULT_SOURCE 1

#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include <zephyr/sys/libc-hooks.h>

/* the result is the caller's to read, in user mode too: C library state */
Z_LIBC_DATA int h_errno;

static Z_LIBC_DATA struct hostent hostent_result;
static Z_LIBC_DATA char hostent_name[NI_MAXHOST];
static Z_LIBC_DATA char hostent_addr[sizeof(struct in6_addr)];
static Z_LIBC_DATA char *hostent_addr_list[2];
static Z_LIBC_DATA char *hostent_aliases[1];

/* the one static result: a name and the one address of the family */
static struct hostent *hostent_fill(const char *name, int family, const void *addr, size_t len)
{
	strncpy(hostent_name, name, sizeof(hostent_name) - 1);
	hostent_name[sizeof(hostent_name) - 1] = '\0';
	memcpy(hostent_addr, addr, len);
	hostent_addr_list[0] = hostent_addr;
	hostent_addr_list[1] = NULL;
	hostent_aliases[0] = NULL;
	hostent_result = (struct hostent){
		.h_name = hostent_name,
		.h_aliases = hostent_aliases,
		.h_addrtype = family,
		.h_length = (int)len,
		.h_addr_list = hostent_addr_list,
	};

	return &hostent_result;
}

struct hostent *gethostbyname(const char *name)
{
	struct addrinfo *res = NULL;
	struct hostent *he;
	const void *addr;
	size_t len;
	int ret;

	ret = getaddrinfo(name, NULL, NULL, &res);
	if (ret != 0) {
		switch (ret) {
		case EAI_AGAIN:
			h_errno = TRY_AGAIN;
			break;
		case EAI_NONAME:
		case EAI_NODATA:
			h_errno = HOST_NOT_FOUND;
			break;
		default:
			h_errno = NO_RECOVERY;
			break;
		}
		return NULL;
	}

	if (res->ai_family == AF_INET6) {
		addr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
		len = sizeof(struct in6_addr);
	} else {
		addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
		len = sizeof(struct in_addr);
	}
	he = hostent_fill((res->ai_canonname != NULL) ? res->ai_canonname : name, res->ai_family, addr,
			  len);
	freeaddrinfo(res);

	return he;
}

/* the hosts database names the address, or nothing does */
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type)
{
	struct hostent *he, *found = NULL;

	if (!(((type == AF_INET) && (len == sizeof(struct in_addr))) ||
	      ((type == AF_INET6) && (len == sizeof(struct in6_addr))))) {
		h_errno = NO_RECOVERY;
		return NULL;
	}

	sethostent(1);
	while ((found == NULL) && ((he = gethostent()) != NULL)) {
		if ((he->h_addrtype != type) || (he->h_length != (int)len)) {
			continue;
		}
		for (char **a = he->h_addr_list; (found == NULL) && (*a != NULL); a++) {
			if (memcmp(*a, addr, len) == 0) {
				found = hostent_fill(he->h_name, type, addr, len);
			}
		}
	}
	endhostent();

	if (found == NULL) {
		h_errno = HOST_NOT_FOUND;
	}

	return found;
}

const char *hstrerror(int err)
{
	switch (err) {
	case 0:
		return "Resolver error 0 (no error)";
	case HOST_NOT_FOUND:
		return "Unknown host";
	case TRY_AGAIN:
		return "Host name lookup failure";
	case NO_RECOVERY:
		return "Unknown server error";
	case NO_DATA:
		return "No address associated with name";
	default:
		return "Unknown resolver error";
	}
}
