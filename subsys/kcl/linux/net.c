/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Sockets, over the network stack's. Option, flag and poll numbering is
 * Linux's already; address families and address structures are converted.
 * A Linux "ping socket" (SOCK_DGRAM over ICMP) is emulated on a raw ICMP
 * socket: the executable builds the echo request itself, and on delivery
 * the IPv4 header is stripped, the TTL handed over as a control message,
 * and only echo replies answering this socket are returned.
 */

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include "kcl_linux.h"

#define LINUX_AF_UNSPEC 0
#define LINUX_AF_UNIX 1
#define LINUX_AF_INET 2
#define LINUX_AF_INET6 10
#define LINUX_SOCK_TYPE_MASK 0xf
#define LINUX_SOCK_NONBLOCK 04000
#define LINUX_SOCK_CLOEXEC 02000000
#define LINUX_SOL_SOCKET 1
#define LINUX_SO_MARK 36
#define LINUX_IP_TTL 2
#define LINUX_IP_RECVTTL 12
#define LINUX_IPV6_RECVHOPLIMIT 51
#define LINUX_ICMP_ECHOREPLY 0
#define LINUX_ICMPV6_ECHO_REPLY 129
#define LINUX_MSG_TRUNC 0x20

struct linux_sockaddr_in {
	uint16_t sin_family;
	uint16_t sin_port;
	uint32_t sin_addr;
	uint8_t sin_zero[8];
};

struct linux_sockaddr_in6 {
	uint16_t sin6_family;
	uint16_t sin6_port;
	uint32_t sin6_flowinfo;
	uint8_t sin6_addr[16];
	uint32_t sin6_scope_id;
};

struct linux_msghdr {
	void *msg_name;
	int msg_namelen;
	struct zkcl_linux_iovec *msg_iov;
	size_t msg_iovlen;
	void *msg_control;
	size_t msg_controllen;
	unsigned int msg_flags;
};

struct linux_cmsghdr {
	size_t cmsg_len;
	int cmsg_level;
	int cmsg_type;
};

struct linux_pollfd {
	int fd;
	short events;
	short revents;
};

static long result(long ret)
{
	return (ret < 0) ? -errno : ret;
}

struct zkcl_linux_sock *zkcl_linux_sock_find(int fd)
{
	struct zkcl_linux_process *p = zkcl_linux_current();

	for (size_t i = 0; (p != NULL) && (i < ARRAY_SIZE(p->socks)); i++) {
		if (p->socks[i].fd == fd) {
			return &p->socks[i];
		}
	}

	return NULL;
}

void zkcl_linux_sock_forget(int fd)
{
	struct zkcl_linux_sock *s = zkcl_linux_sock_find(fd);

	if (s != NULL) {
		s->fd = -1;
	}
}

static struct zkcl_linux_sock *sock_remember(int fd, uint8_t family)
{
	struct zkcl_linux_process *p = zkcl_linux_current();

	zkcl_linux_sock_forget(fd);
	for (size_t i = 0; (p != NULL) && (i < ARRAY_SIZE(p->socks)); i++) {
		if (p->socks[i].fd < 0) {
			p->socks[i] = (struct zkcl_linux_sock){.fd = fd, .family = family};
			return &p->socks[i];
		}
	}

	return NULL;
}

static int family_to_net(int family)
{
	switch (family) {
	case LINUX_AF_UNSPEC:
		return NET_AF_UNSPEC;
	case LINUX_AF_INET:
		return NET_AF_INET;
	case LINUX_AF_INET6:
		return NET_AF_INET6;
	default:
		return -1;
	}
}

/* a Linux socket address from the executable into the stack's; 0 or -errno */
static int sockaddr_to_net(const void *uaddr, size_t ulen, struct net_sockaddr_storage *out,
			   net_socklen_t *outlen)
{
	uint16_t family;

	if ((ulen < sizeof(family)) || (zkcl_linux_user_ok(uaddr, ulen, false) != 0)) {
		return -EFAULT;
	}
	memcpy(&family, uaddr, sizeof(family));
	memset(out, 0, sizeof(*out));
	if ((family == LINUX_AF_INET) && (ulen >= sizeof(struct linux_sockaddr_in))) {
		struct linux_sockaddr_in in;
		struct net_sockaddr_in *nin = (struct net_sockaddr_in *)out;

		memcpy(&in, uaddr, sizeof(in));
		nin->sin_family = NET_AF_INET;
		nin->sin_port = in.sin_port;
		nin->sin_addr.s_addr = in.sin_addr;
		*outlen = sizeof(*nin);
		return 0;
	}
	if ((family == LINUX_AF_INET6) && (ulen >= sizeof(struct linux_sockaddr_in6))) {
		struct linux_sockaddr_in6 in6;
		struct net_sockaddr_in6 *nin6 = (struct net_sockaddr_in6 *)out;

		memcpy(&in6, uaddr, sizeof(in6));
		nin6->sin6_family = NET_AF_INET6;
		nin6->sin6_port = in6.sin6_port;
		memcpy(nin6->sin6_addr.s6_addr, in6.sin6_addr, sizeof(in6.sin6_addr));
		nin6->sin6_scope_id = in6.sin6_scope_id;
		nin6->sin6_flowinfo = in6.sin6_flowinfo;
		*outlen = sizeof(*nin6);
		return 0;
	}

	return -EAFNOSUPPORT;
}

/* the stack's socket address to the executable, Linux style, as much as fits */
static int sockaddr_to_user(const struct net_sockaddr *addr, void *uaddr, int avail, int *outlen)
{
	union {
		struct linux_sockaddr_in in;
		struct linux_sockaddr_in6 in6;
	} u;
	size_t len;

	memset(&u, 0, sizeof(u));
	if (addr->sa_family == NET_AF_INET) {
		const struct net_sockaddr_in *nin = (const struct net_sockaddr_in *)addr;

		u.in.sin_family = LINUX_AF_INET;
		u.in.sin_port = nin->sin_port;
		u.in.sin_addr = nin->sin_addr.s_addr;
		len = sizeof(u.in);
	} else if (addr->sa_family == NET_AF_INET6) {
		const struct net_sockaddr_in6 *nin6 = (const struct net_sockaddr_in6 *)addr;

		u.in6.sin6_family = LINUX_AF_INET6;
		u.in6.sin6_port = nin6->sin6_port;
		memcpy(u.in6.sin6_addr, nin6->sin6_addr.s6_addr, sizeof(u.in6.sin6_addr));
		u.in6.sin6_scope_id = nin6->sin6_scope_id;
		u.in6.sin6_flowinfo = nin6->sin6_flowinfo;
		len = sizeof(u.in6);
	} else {
		return -EAFNOSUPPORT;
	}
	if ((avail > 0) && (uaddr != NULL)) {
		size_t n = MIN((size_t)avail, len);

		if (zkcl_linux_user_ok(uaddr, n, true) != 0) {
			return -EFAULT;
		}
		memcpy(uaddr, &u, n);
	}
	*outlen = (int)len;

	return 0;
}

/* the same, with the length in and out of the executable's memory */
static int sockaddr_to_user_len(const struct net_sockaddr *addr, void *uaddr, int *ulen)
{
	int avail;
	int ret;

	if (zkcl_linux_user_ok(ulen, sizeof(*ulen), true) != 0) {
		return -EFAULT;
	}
	memcpy(&avail, ulen, sizeof(avail));
	ret = sockaddr_to_user(addr, uaddr, avail, &avail);
	if (ret == 0) {
		memcpy(ulen, &avail, sizeof(avail));
	}

	return ret;
}

ZKCL_LINUX_IMPL(socket)(int family, int type, int protocol)
{
	int nfamily = family_to_net(family);
	int ntype = type & LINUX_SOCK_TYPE_MASK;
	bool icmp_dgram = false;
	struct zkcl_linux_sock *s;
	int fd;

	if (nfamily < 0) {
		return -EAFNOSUPPORT;
	}
	if ((ntype == NET_SOCK_DGRAM) &&
	    (((nfamily == NET_AF_INET) && (protocol == NET_IPPROTO_ICMP)) ||
	     ((nfamily == NET_AF_INET6) && (protocol == NET_IPPROTO_ICMPV6)))) {
		/* no unprivileged ICMP sockets: a raw one, with the delivery adjusted */
		ntype = NET_SOCK_RAW;
		icmp_dgram = true;
	}
	fd = zsock_socket(nfamily, ntype, protocol);
	if (fd < 0) {
		return -errno;
	}
	s = sock_remember(fd, nfamily);
	if (s != NULL) {
		s->icmp_dgram = icmp_dgram;
	}
	if ((type & LINUX_SOCK_NONBLOCK) != 0) {
		(void)zvfs_fcntl(fd, ZVFS_F_SETFL, ZVFS_O_NONBLOCK);
	}
	if ((type & LINUX_SOCK_CLOEXEC) != 0) {
		(void)zvfs_fcntl(fd, ZVFS_F_SETFD, ZVFS_FD_CLOEXEC);
	}

	return fd;
}

ZKCL_LINUX_IMPL(bind)(int fd, struct zkcl_linux_sockaddr *umyaddr, int addrlen)
{
	struct net_sockaddr_storage addr;
	net_socklen_t len;
	int ret = sockaddr_to_net(umyaddr, addrlen, &addr, &len);

	if (ret != 0) {
		return ret;
	}

	return result(zsock_bind(fd, (struct net_sockaddr *)&addr, len));
}

ZKCL_LINUX_IMPL(connect)(int fd, struct zkcl_linux_sockaddr *uservaddr, int addrlen)
{
	struct net_sockaddr_storage addr;
	net_socklen_t len;
	int ret = sockaddr_to_net(uservaddr, addrlen, &addr, &len);

	if (ret != 0) {
		return ret;
	}

	return result(zsock_connect(fd, (struct net_sockaddr *)&addr, len));
}

ZKCL_LINUX_IMPL(listen)(int fd, int backlog)
{
	return result(zsock_listen(fd, backlog));
}

static long accept4_impl(int fd, struct zkcl_linux_sockaddr *upeer_sockaddr,
				       int *upeer_addrlen, int flags)
{
	struct net_sockaddr_storage addr;
	net_socklen_t len = sizeof(addr);
	struct zkcl_linux_sock *s = zkcl_linux_sock_find(fd);
	int newfd = zsock_accept(fd, (struct net_sockaddr *)&addr, &len);

	if (newfd < 0) {
		return -errno;
	}
	(void)sock_remember(newfd, (s != NULL) ? s->family : addr.ss_family);
	if ((flags & LINUX_SOCK_NONBLOCK) != 0) {
		(void)zvfs_fcntl(newfd, ZVFS_F_SETFL, ZVFS_O_NONBLOCK);
	}
	if ((flags & LINUX_SOCK_CLOEXEC) != 0) {
		(void)zvfs_fcntl(newfd, ZVFS_F_SETFD, ZVFS_FD_CLOEXEC);
	}
	if ((upeer_sockaddr != NULL) && (upeer_addrlen != NULL)) {
		(void)sockaddr_to_user_len((struct net_sockaddr *)&addr, upeer_sockaddr, upeer_addrlen);
	}

	return newfd;
}

ZKCL_LINUX_IMPL(accept4)(int fd, struct zkcl_linux_sockaddr *upeer_sockaddr,
				       int *upeer_addrlen, int flags)
{
	return accept4_impl(fd, upeer_sockaddr, upeer_addrlen, flags);
}

ZKCL_LINUX_IMPL(accept)(int fd, struct zkcl_linux_sockaddr *upeer_sockaddr,
				      int *upeer_addrlen)
{
	return accept4_impl(fd, upeer_sockaddr, upeer_addrlen, 0);
}

ZKCL_LINUX_IMPL(shutdown)(int fd, int how)
{
	return result(zsock_shutdown(fd, how));
}

static long name_of(int fd, struct zkcl_linux_sockaddr *usockaddr, int *usockaddr_len,
		    bool peer)
{
	struct net_sockaddr_storage addr;
	net_socklen_t len = sizeof(addr);
	int ret = peer ? zsock_getpeername(fd, (struct net_sockaddr *)&addr, &len)
		       : zsock_getsockname(fd, (struct net_sockaddr *)&addr, &len);

	if (ret < 0) {
		return -errno;
	}

	return sockaddr_to_user_len((struct net_sockaddr *)&addr, usockaddr, usockaddr_len);
}

ZKCL_LINUX_IMPL(getsockname)(int fd, struct zkcl_linux_sockaddr *usockaddr,
					   int *usockaddr_len)
{
	return name_of(fd, usockaddr, usockaddr_len, false);
}

ZKCL_LINUX_IMPL(getpeername)(int fd, struct zkcl_linux_sockaddr *usockaddr,
					   int *usockaddr_len)
{
	return name_of(fd, usockaddr, usockaddr_len, true);
}

ZKCL_LINUX_IMPL(setsockopt)(int fd, int level, int optname, char *optval,
					  int optlen)
{
	struct zkcl_linux_sock *s = zkcl_linux_sock_find(fd);

	if ((optlen < 0) || (zkcl_linux_user_ok(optval, optlen, false) != 0)) {
		return -EFAULT;
	}
	if ((level == LINUX_SOL_SOCKET) && (optname == LINUX_SO_MARK)) {
		return 0;
	}
	if ((s != NULL) && (((level == NET_IPPROTO_IP) && (optname == LINUX_IP_RECVTTL)) ||
			    ((level == NET_IPPROTO_IPV6) && (optname == LINUX_IPV6_RECVHOPLIMIT)))) {
		int on = 0;

		memcpy(&on, optval, MIN(sizeof(on), (size_t)optlen));
		s->recvttl = (on != 0);
		if (s->icmp_dgram) {
			/* delivered from the header the emulation strips */
			return 0;
		}
	}

	return result(zsock_setsockopt(fd, level, optname, optval, optlen));
}

ZKCL_LINUX_IMPL(getsockopt)(int fd, int level, int optname, char *optval,
					  int *optlen)
{
	net_socklen_t len;
	int ulen;
	int ret;

	if ((zkcl_linux_user_ok(optlen, sizeof(*optlen), true) != 0)) {
		return -EFAULT;
	}
	memcpy(&ulen, optlen, sizeof(ulen));
	if ((ulen < 0) || (zkcl_linux_user_ok(optval, ulen, true) != 0)) {
		return -EFAULT;
	}
	len = ulen;
	ret = zsock_getsockopt(fd, level, optname, optval, &len);
	if (ret < 0) {
		return -errno;
	}
	ulen = len;
	memcpy(optlen, &ulen, sizeof(ulen));

	return 0;
}

ZKCL_LINUX_IMPL(sendto)(int fd, void *buff, size_t len, unsigned int flags,
				      struct zkcl_linux_sockaddr *addr, int addr_len)
{
	struct zkcl_linux_sock *s = zkcl_linux_sock_find(fd);
	struct net_sockaddr_storage naddr;
	net_socklen_t nlen = 0;
	int ret;

	if (zkcl_linux_user_ok(buff, len, false) != 0) {
		return -EFAULT;
	}
	if (addr != NULL) {
		ret = sockaddr_to_net(addr, addr_len, &naddr, &nlen);
		if (ret != 0) {
			return ret;
		}
	}
	if ((s != NULL) && s->icmp_dgram && (len >= 8)) {
		/* the echo id the executable chose: replies are matched on it */
		memcpy(&s->echo_id, (uint8_t *)buff + 4, sizeof(s->echo_id));
		if (addr != NULL) {
			BUILD_ASSERT(sizeof(s->peer) >= sizeof(naddr));
			memcpy(s->peer, &naddr, sizeof(naddr));
		}
	}

	return result(zsock_sendto(fd, buff, len, flags,
				   (addr != NULL) ? (struct net_sockaddr *)&naddr : NULL, nlen));
}

/* a Linux control message with one int, if there is room */
static size_t cmsg_int(void *control, size_t controllen, int level, int type, int value)
{
	struct linux_cmsghdr hdr = {
		.cmsg_len = sizeof(hdr) + sizeof(value),
		.cmsg_level = level,
		.cmsg_type = type,
	};
	size_t space = ROUND_UP(hdr.cmsg_len, sizeof(size_t));

	if ((control == NULL) || (controllen < space) ||
	    (zkcl_linux_user_ok(control, space, true) != 0)) {
		return 0;
	}
	memcpy(control, &hdr, sizeof(hdr));
	memcpy((uint8_t *)control + sizeof(hdr), &value, sizeof(value));

	return space;
}

/*
 * Receive into a kernel buffer; for a ping socket, wait for an echo reply
 * answering this socket and hand back the ICMP message alone, with its TTL.
 */
static long recv_into(int fd, struct zkcl_linux_sock *s, uint8_t *buf, size_t size,
		      unsigned int flags, struct net_sockaddr_storage *from,
		      net_socklen_t *fromlen, int *ttl)
{
	*ttl = -1;
	while (true) {
		ssize_t n;
		size_t hl = 0;
		uint8_t type;
		uint16_t id;

		*fromlen = sizeof(*from);
		n = zsock_recvfrom(fd, buf, size, flags, (struct net_sockaddr *)from, fromlen);
		if (n < 0) {
			return -errno;
		}
		if ((s == NULL) || !s->icmp_dgram) {
			return n;
		}
		if (s->family == NET_AF_INET) {
			if ((n < 20) || ((buf[0] >> 4) != 4)) {
				continue;
			}
			hl = (buf[0] & 0xf) * 4;
			*ttl = buf[8];
			type = LINUX_ICMP_ECHOREPLY;
		} else {
			type = LINUX_ICMPV6_ECHO_REPLY;
			*ttl = 64;
		}
		if ((size_t)n < hl + 8) {
			continue;
		}
		memcpy(&id, buf + hl + 4, sizeof(id));
		if ((buf[hl] != type) || (id != s->echo_id)) {
			/* another message on the raw socket: not for this one */
			continue;
		}
		/* the raw socket names no source: the header's, else the peer asked */
		if (s->family == NET_AF_INET) {
			struct net_sockaddr_in *src = (struct net_sockaddr_in *)from;

			memset(from, 0, sizeof(*from));
			src->sin_family = NET_AF_INET;
			memcpy(&src->sin_addr.s_addr, buf + 12, sizeof(src->sin_addr.s_addr));
			*fromlen = sizeof(*src);
		} else if (from->ss_family != NET_AF_INET6) {
			memcpy(from, s->peer, sizeof(*from));
			*fromlen = sizeof(struct net_sockaddr_in6);
		}
		memmove(buf, buf + hl, n - hl);
		return n - hl;
	}
}

ZKCL_LINUX_IMPL(recvfrom)(int fd, void *ubuf, size_t size, unsigned int flags,
					struct zkcl_linux_sockaddr *addr, int *addr_len)
{
	struct zkcl_linux_sock *s = zkcl_linux_sock_find(fd);
	struct net_sockaddr_storage from;
	net_socklen_t fromlen;
	uint8_t *buf;
	int ttl;
	long n;

	if (zkcl_linux_user_ok(ubuf, size, true) != 0) {
		return -EFAULT;
	}
	if ((s == NULL) || !s->icmp_dgram) {
		n = zsock_recvfrom(fd, ubuf, size, flags, (struct net_sockaddr *)&from,
				   &(net_socklen_t){sizeof(from)});
		if (n < 0) {
			return -errno;
		}
	} else {
		buf = k_malloc(size + 64);
		if (buf == NULL) {
			return -ENOMEM;
		}
		n = recv_into(fd, s, buf, size + 64, flags, &from, &fromlen, &ttl);
		if (n > 0) {
			memcpy(ubuf, buf, MIN((size_t)n, size));
		}
		k_free(buf);
		if (n < 0) {
			return n;
		}
		n = MIN((size_t)n, size);
	}
	if ((addr != NULL) && (addr_len != NULL)) {
		(void)sockaddr_to_user_len((struct net_sockaddr *)&from, addr, addr_len);
	}

	return n;
}

ZKCL_LINUX_IMPL(recvmsg)(int fd, struct zkcl_linux_user_msghdr *umsg,
				       unsigned int flags)
{
	struct zkcl_linux_sock *s = zkcl_linux_sock_find(fd);
	struct linux_msghdr msg;
	struct net_sockaddr_storage from;
	net_socklen_t fromlen;
	uint8_t *buf;
	size_t size = 0;
	size_t off = 0;
	int ttl;
	long n;

	if (zkcl_linux_user_ok(umsg, sizeof(msg), true) != 0) {
		return -EFAULT;
	}
	memcpy(&msg, umsg, sizeof(msg));
	if ((msg.msg_iovlen > 8) ||
	    (zkcl_linux_user_ok(msg.msg_iov, msg.msg_iovlen * sizeof(*msg.msg_iov), false) != 0)) {
		return -EFAULT;
	}
	for (size_t i = 0; i < msg.msg_iovlen; i++) {
		if (zkcl_linux_user_ok(msg.msg_iov[i].iov_base, msg.msg_iov[i].iov_len, true) != 0) {
			return -EFAULT;
		}
		size += msg.msg_iov[i].iov_len;
	}
	buf = k_malloc(size + 64);
	if (buf == NULL) {
		return -ENOMEM;
	}
	n = recv_into(fd, s, buf, size + 64, flags, &from, &fromlen, &ttl);
	if (n < 0) {
		k_free(buf);
		return n;
	}
	for (size_t i = 0; (i < msg.msg_iovlen) && (off < (size_t)n); i++) {
		size_t chunk = MIN(msg.msg_iov[i].iov_len, (size_t)n - off);

		memcpy(msg.msg_iov[i].iov_base, buf + off, chunk);
		off += chunk;
	}
	k_free(buf);
	msg.msg_flags = ((size_t)n > size) ? LINUX_MSG_TRUNC : 0;
	if ((msg.msg_name != NULL) && (msg.msg_namelen > 0)) {
		int namelen;

		if (sockaddr_to_user((struct net_sockaddr *)&from, msg.msg_name, msg.msg_namelen,
				     &namelen) == 0) {
			msg.msg_namelen = namelen;
		}
	}
	if ((s != NULL) && s->recvttl && (ttl >= 0)) {
		/* as IP_TTL for both families: that is what asked for it with IP_RECVTTL */
		msg.msg_controllen = cmsg_int(msg.msg_control, msg.msg_controllen, NET_IPPROTO_IP,
					      LINUX_IP_TTL, ttl);
	} else {
		msg.msg_controllen = 0;
	}
	memcpy(umsg, &msg, sizeof(msg));

	return MIN((size_t)n, size);
}

ZKCL_LINUX_IMPL(sendmsg)(int fd, struct zkcl_linux_user_msghdr *umsg,
				       unsigned int flags)
{
	struct linux_msghdr msg;
	struct net_sockaddr_storage naddr;
	net_socklen_t nlen = 0;
	uint8_t *buf;
	size_t size = 0;
	size_t off = 0;
	long ret;

	if (zkcl_linux_user_ok(umsg, sizeof(msg), false) != 0) {
		return -EFAULT;
	}
	memcpy(&msg, umsg, sizeof(msg));
	if ((msg.msg_iovlen > 8) ||
	    (zkcl_linux_user_ok(msg.msg_iov, msg.msg_iovlen * sizeof(*msg.msg_iov), false) != 0)) {
		return -EFAULT;
	}
	for (size_t i = 0; i < msg.msg_iovlen; i++) {
		if (zkcl_linux_user_ok(msg.msg_iov[i].iov_base, msg.msg_iov[i].iov_len, false) != 0) {
			return -EFAULT;
		}
		size += msg.msg_iov[i].iov_len;
	}
	if (msg.msg_name != NULL) {
		ret = sockaddr_to_net(msg.msg_name, msg.msg_namelen, &naddr, &nlen);
		if (ret != 0) {
			return ret;
		}
	}
	buf = k_malloc(MAX(size, 1));
	if (buf == NULL) {
		return -ENOMEM;
	}
	for (size_t i = 0; i < msg.msg_iovlen; i++) {
		memcpy(buf + off, msg.msg_iov[i].iov_base, msg.msg_iov[i].iov_len);
		off += msg.msg_iov[i].iov_len;
	}
	{
		struct zkcl_linux_sock *s = zkcl_linux_sock_find(fd);

		if ((s != NULL) && s->icmp_dgram && (size >= 8)) {
			memcpy(&s->echo_id, buf + 4, sizeof(s->echo_id));
			if (msg.msg_name != NULL) {
				memcpy(s->peer, &naddr, sizeof(naddr));
			}
		}
	}
	ret = result(zsock_sendto(fd, buf, size, flags,
				  (msg.msg_name != NULL) ? (struct net_sockaddr *)&naddr : NULL, nlen));
	k_free(buf);

	return ret;
}

static long poll_common(struct zkcl_linux_pollfd *ufds, unsigned int nfds, int timeout_ms)
{
	struct zvfs_pollfd fds[16];
	struct linux_pollfd lfds[16];
	long ret;

	if (nfds > ARRAY_SIZE(fds)) {
		return -EINVAL;
	}
	if (zkcl_linux_user_ok(ufds, nfds * sizeof(*lfds), true) != 0) {
		return -EFAULT;
	}
	memcpy(lfds, ufds, nfds * sizeof(*lfds));
	for (unsigned int i = 0; i < nfds; i++) {
		fds[i].fd = lfds[i].fd;
		fds[i].events = lfds[i].events;
		fds[i].revents = 0;
	}
	ret = result(zvfs_poll(fds, nfds, timeout_ms));
	for (unsigned int i = 0; i < nfds; i++) {
		lfds[i].revents = fds[i].revents;
	}
	memcpy(ufds, lfds, nfds * sizeof(*lfds));

	return ret;
}

ZKCL_LINUX_IMPL(poll)(struct zkcl_linux_pollfd *ufds, unsigned int nfds,
				    int timeout_msecs)
{
	return poll_common(ufds, nfds, timeout_msecs);
}

ZKCL_LINUX_IMPL(ppoll)(struct zkcl_linux_pollfd *ufds, unsigned int nfds,
				     struct zkcl_linux_timespec *tsp, const zkcl_linux_sigset_t *sigmask,
				     size_t sigsetsize)
{
	int timeout_ms = -1;

	ARG_UNUSED(sigmask);
	ARG_UNUSED(sigsetsize);
	if (tsp != NULL) {
		struct zkcl_linux_timespec ts;

		if (zkcl_linux_user_ok(tsp, sizeof(ts), false) != 0) {
			return -EFAULT;
		}
		memcpy(&ts, tsp, sizeof(ts));
		timeout_ms = (int)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
	}

	return poll_common(ufds, nfds, timeout_ms);
}
