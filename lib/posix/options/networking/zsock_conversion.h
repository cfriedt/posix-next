/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr zsock_* ↔ POSIX network struct conversion framework.
 *
 * Each conversion pair follows the same pattern:
 *
 * 1. @c *_layout_eq() — runtime check of container @c sizeof and shared-field
 *    offset/size (fields present in only one type are skipped).
 * 2. Slow path — field-wise copy into @p buf when layouts differ.
 * 3. Fast path — cast @p in to the output pointer when @c *_layout_eq() is true.
 *
 * Typed converters are grouped in sections below (sockaddr first; msghdr, addrinfo,
 * iovec, ipv6_mreq, … as needed). Generic header-only types (e.g.
 * struct sockaddr) dispatch to the typed inlines.
 *
 * POSIX @c poll() uses @c zvfs_pollfd (POSIX_DEVICE_IO); @c poll.h aliases
 * @c pollfd to @c zvfs_pollfd — not converted here. Zephyr @c zsock_pollfd is
 * the same type (@c socket_poll.h).
 *
 * Goals (ideal case — all true when Zephyr and active libc layouts match Linux):
 * - POSIX correctness
 * - Linux-equivalent layouts in the posix-next reference headers
 * - Zero-copy fast path when layouts are identical
 */

#ifndef ZEPHYR_POSIX_OPTIONS_NETWORKING_ZSOCK_CONVERSION_H_
#define ZEPHYR_POSIX_OPTIONS_NETWORKING_ZSOCK_CONVERSION_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/**
 * @brief Compare offset and size of one field in a zsock_* vs POSIX struct pair.
 *
 * Fields present in only one type are omitted from the caller's comparison list.
 */
#define POSIX_NET_FIELD_LAYOUT_EQ(native_type, posix_type, native_field, posix_field)              \
	((sizeof(((native_type *)0)->native_field) == sizeof(((posix_type *)0)->posix_field)) &&   \
	 (offsetof(native_type, native_field) == offsetof(posix_type, posix_field)))

/** @brief True when @p native_type and @p posix_type have identical object size. */
#define POSIX_NET_STRUCT_LAYOUT_EQ(native_type, posix_type)                                        \
	(sizeof(native_type) == sizeof(posix_type))

/*
 * Future conversion sections (same pattern as sockaddr):
 *   zsock_ipv6_mreq       ↔ struct ipv6_mreq
 *
 * pollfd / zvfs_pollfd: POSIX_DEVICE_IO only; poll.h already #define pollfd
 * zvfs_pollfd (same layout as zsock_pollfd). No networking converter expected.
 */

#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <net/if.h>
#include <netdb.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/__assert.h>

#ifndef zsock_in_addr
#define zsock_in_addr in_addr
#endif

/**
 * @brief Stack buffer for wrapper ↔ zsock conversion (POSIX sockaddr_storage size).
 *
 * Larger than @c struct zsock_sockaddr_storage, which tracks @c NET_SOCKADDR_MAX_SIZE
 * (often IPv4/IPv6 only). Required so AF_UNIX path copies do not overflow under
 * picolibc _FORTIFY_SOURCE.
 */
struct posix_zsock_sockaddr_buf {
	uint8_t bytes[sizeof(struct sockaddr_storage)];
};

static ALWAYS_INLINE struct zsock_sockaddr *posix_zsock_sa_buf(struct posix_zsock_sockaddr_buf *buf)
{
	return (struct zsock_sockaddr *)buf->bytes;
}

/**
 * @brief View a generic or storage sockaddr as @c struct sockaddr_in.
 *
 * @param addr Pointer to @c sockaddr or the @c sockaddr prefix of @c sockaddr_storage.
 * @return Typed IPv4 socket address (same storage as @p addr).
 */
static ALWAYS_INLINE struct sockaddr_in *posix_sin(struct sockaddr *addr)
{
	__ASSERT(addr != NULL, "addr must not be NULL");
	__ASSERT(addr->sa_family == AF_INET, "expected AF_INET, got %u",
		 (unsigned int)addr->sa_family);
	return (struct sockaddr_in *)addr;
}

/**
 * @brief Const variant of posix_sin().
 */
static ALWAYS_INLINE const struct sockaddr_in *posix_sin_const(const struct sockaddr *addr)
{
	__ASSERT(addr != NULL, "addr must not be NULL");
	__ASSERT(addr->sa_family == AF_INET, "expected AF_INET, got %u",
		 (unsigned int)addr->sa_family);
	return (const struct sockaddr_in *)addr;
}

/**
 * @brief View @c sockaddr_storage as @c struct sockaddr_in.
 */
static ALWAYS_INLINE struct sockaddr_in *posix_sin_storage(struct sockaddr_storage *ss)
{
	return posix_sin((struct sockaddr *)ss);
}

/**
 * @brief View a generic or storage sockaddr as @c struct sockaddr_in6.
 *
 * @param addr Pointer to @c sockaddr or the @c sockaddr prefix of @c sockaddr_storage.
 * @return Typed IPv6 socket address (same storage as @p addr).
 */
static ALWAYS_INLINE struct sockaddr_in6 *posix_sin6(struct sockaddr *addr)
{
	__ASSERT(addr != NULL, "addr must not be NULL");
	__ASSERT(addr->sa_family == AF_INET6, "expected AF_INET6, got %u",
		 (unsigned int)addr->sa_family);
	return (struct sockaddr_in6 *)addr;
}

/**
 * @brief Const variant of posix_sin6().
 */
static ALWAYS_INLINE const struct sockaddr_in6 *posix_sin6_const(const struct sockaddr *addr)
{
	__ASSERT(addr != NULL, "addr must not be NULL");
	__ASSERT(addr->sa_family == AF_INET6, "expected AF_INET6, got %u",
		 (unsigned int)addr->sa_family);
	return (const struct sockaddr_in6 *)addr;
}

/**
 * @brief View @c sockaddr_storage as @c struct sockaddr_in6.
 */
static ALWAYS_INLINE struct sockaddr_in6 *posix_sin6_storage(struct sockaddr_storage *ss)
{
	return posix_sin6((struct sockaddr *)ss);
}

/**
 * @brief View a generic zsock_sockaddr as @c struct zsock_sockaddr_in.
 */
static ALWAYS_INLINE struct zsock_sockaddr_in *zsock_sin(struct zsock_sockaddr *addr)
{
	__ASSERT(addr != NULL, "addr must not be NULL");
	__ASSERT(addr->sa_family == AF_INET, "expected AF_INET, got %u",
		 (unsigned int)addr->sa_family);
	return (struct zsock_sockaddr_in *)addr;
}

/**
 * @brief Const variant of zsock_sin().
 */
static ALWAYS_INLINE const struct zsock_sockaddr_in *
zsock_sin_const(const struct zsock_sockaddr *addr)
{
	__ASSERT(addr != NULL, "addr must not be NULL");
	__ASSERT(addr->sa_family == AF_INET, "expected AF_INET, got %u",
		 (unsigned int)addr->sa_family);
	return (const struct zsock_sockaddr_in *)addr;
}

/**
 * @brief View a generic zsock_sockaddr as @c struct zsock_sockaddr_in6.
 */
static ALWAYS_INLINE struct zsock_sockaddr_in6 *zsock_sin6(struct zsock_sockaddr *addr)
{
	__ASSERT(addr != NULL, "addr must not be NULL");
	__ASSERT(addr->sa_family == AF_INET6, "expected AF_INET6, got %u",
		 (unsigned int)addr->sa_family);
	return (struct zsock_sockaddr_in6 *)addr;
}

/**
 * @brief Const variant of zsock_sin6().
 */
static ALWAYS_INLINE const struct zsock_sockaddr_in6 *
zsock_sin6_const(const struct zsock_sockaddr *addr)
{
	__ASSERT(addr != NULL, "addr must not be NULL");
	__ASSERT(addr->sa_family == AF_INET6, "expected AF_INET6, got %u",
		 (unsigned int)addr->sa_family);
	return (const struct zsock_sockaddr_in6 *)addr;
}

/*
 * struct in_addr / struct zsock_in_addr: POSIX uses a union; Zephyr matches the
 * Linux layout (s_addr at the same offset). glibc may expose extra union members
 * via macros — those are not compared here.
 */
static ALWAYS_INLINE bool posix_in_addr_layout_eq(void)
{
	return POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_in_addr, struct in_addr) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_in_addr, struct in_addr, s_addr, s_addr);
}

/*
 * sockaddr_in: POSIX adds sin_zero[8] (Linux ABI tail padding). That field is
 * intentionally excluded from layout comparison; equal @c sizeof still requires
 * a matching libc layout for the fast-path cast.
 */
static ALWAYS_INLINE bool posix_sockaddr_in_layout_eq(void)
{
	return POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_sockaddr_in, struct sockaddr_in) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in, struct sockaddr_in, sin_family,
					 sin_family) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in, struct sockaddr_in, sin_port,
					 sin_port) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in, struct sockaddr_in, sin_addr,
					 sin_addr);
}

/*
 * sockaddr_in6: POSIX adds sin6_flowinfo; sin6_scope_id width differs on Zephyr
 * (uint8_t) vs POSIX (uint32_t). Fast-path cast only when the active headers match.
 */
static ALWAYS_INLINE bool posix_sockaddr_in6_layout_eq(void)
{
	return POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6,
					 sin6_family, sin6_family) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6, sin6_port,
					 sin6_port) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6, sin6_addr,
					 sin6_addr) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6,
					 sin6_scope_id, sin6_scope_id);
}

static ALWAYS_INLINE bool posix_sockaddr_un_layout_eq(void)
{
	return POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_sockaddr_un, struct sockaddr_un) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_un, struct sockaddr_un, sun_family,
					 sun_family) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_un, struct sockaddr_un, sun_path,
					 sun_path);
}

static ALWAYS_INLINE struct zsock_sockaddr_in *
posix_sockaddr_in_to_zsock(const struct sockaddr_in *in, struct zsock_sockaddr_in *buf,
			   size_t *out_len)
{
	__ASSERT(in != NULL, "in must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");
	__ASSERT(out_len != NULL, "out_len must not be NULL");
	__ASSERT(*out_len >= sizeof(struct zsock_sockaddr_in), "IPv4 output buffer too small");

	if (posix_sockaddr_in_layout_eq()) {
		*out_len = sizeof(struct zsock_sockaddr_in);
		return (struct zsock_sockaddr_in *)in;
	}

	struct zsock_sockaddr_in *out = buf;

	*out = (struct zsock_sockaddr_in){
		.sin_family = in->sin_family,
		.sin_port = in->sin_port,
		.sin_addr.s_addr = in->sin_addr.s_addr,
	};
	*out_len = sizeof(struct zsock_sockaddr_in);
	return out;
}

static ALWAYS_INLINE struct sockaddr_in *
zsock_sockaddr_in_to_posix(const struct zsock_sockaddr_in *in, struct sockaddr_in *buf,
			   size_t *out_len)
{
	__ASSERT(in != NULL, "in must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");
	__ASSERT(out_len != NULL, "out_len must not be NULL");
	__ASSERT(*out_len >= sizeof(struct sockaddr_in), "IPv4 output buffer too small");

	if (posix_sockaddr_in_layout_eq()) {
		*out_len = sizeof(struct sockaddr_in);
		return (struct sockaddr_in *)in;
	}

	struct sockaddr_in *out = buf;

	*out = (struct sockaddr_in){
		.sin_family = in->sin_family,
		.sin_port = in->sin_port,
		.sin_addr.s_addr = in->sin_addr.s_addr,
	};
	*out_len = sizeof(struct sockaddr_in);
	return out;
}

static ALWAYS_INLINE struct zsock_sockaddr_in6 *
posix_sockaddr_in6_to_zsock(const struct sockaddr_in6 *in, struct zsock_sockaddr_in6 *buf,
			    size_t *out_len)
{
	__ASSERT(in != NULL, "in must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");
	__ASSERT(out_len != NULL, "out_len must not be NULL");
	__ASSERT(*out_len >= sizeof(struct zsock_sockaddr_in6), "IPv6 output buffer too small");

	if (posix_sockaddr_in6_layout_eq()) {
		*out_len = sizeof(struct zsock_sockaddr_in6);
		return (struct zsock_sockaddr_in6 *)in;
	}

	struct zsock_sockaddr_in6 *out = buf;

	*out = (struct zsock_sockaddr_in6){
		.sin6_family = in->sin6_family,
		.sin6_port = in->sin6_port,
		.sin6_scope_id = (uint8_t)in->sin6_scope_id,
	};
	memcpy(&out->sin6_addr, &in->sin6_addr, sizeof(out->sin6_addr));
	*out_len = sizeof(struct zsock_sockaddr_in6);
	return out;
}

static ALWAYS_INLINE struct sockaddr_in6 *
zsock_sockaddr_in6_to_posix(const struct zsock_sockaddr_in6 *in, struct sockaddr_in6 *buf,
			    size_t *out_len)
{
	__ASSERT(in != NULL, "in must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");
	__ASSERT(out_len != NULL, "out_len must not be NULL");
	__ASSERT(*out_len >= sizeof(struct sockaddr_in6), "IPv6 output buffer too small");

	if (posix_sockaddr_in6_layout_eq()) {
		*out_len = sizeof(struct sockaddr_in6);
		return (struct sockaddr_in6 *)in;
	}

	struct sockaddr_in6 *out = buf;

	*out = (struct sockaddr_in6){
		.sin6_family = in->sin6_family,
		.sin6_port = in->sin6_port,
		.sin6_flowinfo = 0,
		.sin6_scope_id = in->sin6_scope_id,
	};
	memcpy(&out->sin6_addr, &in->sin6_addr, sizeof(out->sin6_addr));
	*out_len = sizeof(struct sockaddr_in6);
	return out;
}

static ALWAYS_INLINE struct zsock_sockaddr_un *
posix_sockaddr_un_to_zsock(const struct sockaddr_un *in, struct zsock_sockaddr_un *buf,
			   size_t *out_len)
{
	__ASSERT(in != NULL, "in must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");
	__ASSERT(out_len != NULL, "out_len must not be NULL");
	__ASSERT(*out_len >= sizeof(struct zsock_sockaddr_un), "UNIX output buffer too small");

	if (posix_sockaddr_un_layout_eq()) {
		*out_len = sizeof(struct zsock_sockaddr_un);
		return (struct zsock_sockaddr_un *)in;
	}

	struct zsock_sockaddr_un *out = buf;

	*out = (struct zsock_sockaddr_un){
		.sun_family = in->sun_family,
	};
	for (size_t i = 0; i < sizeof(out->sun_path); i++) {
		out->sun_path[i] = in->sun_path[i];
	}
	*out_len = sizeof(struct zsock_sockaddr_un);
	return out;
}

static ALWAYS_INLINE struct sockaddr_un *
zsock_sockaddr_un_to_posix(const struct zsock_sockaddr_un *in, struct sockaddr_un *buf,
			   size_t *out_len)
{
	__ASSERT(in != NULL, "in must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");
	__ASSERT(out_len != NULL, "out_len must not be NULL");
	__ASSERT(*out_len >= sizeof(struct sockaddr_un), "UNIX output buffer too small");

	if (posix_sockaddr_un_layout_eq()) {
		*out_len = sizeof(struct sockaddr_un);
		return (struct sockaddr_un *)in;
	}

	struct sockaddr_un *out = buf;

	*out = (struct sockaddr_un){
		.sun_family = in->sun_family,
	};
	for (size_t i = 0; i < sizeof(out->sun_path); i++) {
		out->sun_path[i] = in->sun_path[i];
	}
	*out_len = sizeof(struct sockaddr_un);
	return out;
}

/**
 * @brief Convert a POSIX sockaddr header to a Zephyr zsock_sockaddr buffer.
 *
 * Generic struct sockaddr is a header-only view: dispatch to the typed inline
 * for the address family. Unsupported families assert at runtime.
 */
static ALWAYS_INLINE struct zsock_sockaddr *posix_sockaddr_to_zsock(const struct sockaddr *addr,
								    size_t addrlen,
								    struct zsock_sockaddr *buf,
								    size_t *out_len)
{
	__ASSERT(addr != NULL, "addr must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");
	__ASSERT(out_len != NULL, "out_len must not be NULL");
	__ASSERT(addrlen >= sizeof(sa_family_t), "addrlen too short");

	switch (addr->sa_family) {
	case AF_INET:
		__ASSERT(addrlen >= sizeof(struct sockaddr_in), "IPv4 addrlen too short");
		return (struct zsock_sockaddr *)posix_sockaddr_in_to_zsock(
			posix_sin_const(addr), (struct zsock_sockaddr_in *)buf, out_len);
	case AF_INET6:
		__ASSERT(addrlen >= sizeof(struct sockaddr_in6), "IPv6 addrlen too short");
		return (struct zsock_sockaddr *)posix_sockaddr_in6_to_zsock(
			posix_sin6_const(addr), (struct zsock_sockaddr_in6 *)buf, out_len);
	case AF_UNIX:
		__ASSERT(addrlen >= sizeof(struct sockaddr_un), "UNIX addrlen too short");
		return (struct zsock_sockaddr *)posix_sockaddr_un_to_zsock(
			(const struct sockaddr_un *)addr, (struct zsock_sockaddr_un *)buf, out_len);
	default:
		__ASSERT(false, "unsupported POSIX sockaddr family %u",
			 (unsigned int)addr->sa_family);
		return NULL;
	}
}

/**
 * @brief Convert a Zephyr zsock_sockaddr to a POSIX sockaddr buffer.
 */
static ALWAYS_INLINE struct sockaddr *zsock_sockaddr_to_posix(const struct zsock_sockaddr *zaddr,
							      size_t zaddrlen, struct sockaddr *buf,
							      size_t *out_len)
{
	__ASSERT(zaddr != NULL, "zaddr must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");
	__ASSERT(out_len != NULL, "out_len must not be NULL");

	switch (zaddr->sa_family) {
	case AF_INET:
		__ASSERT(zaddrlen >= sizeof(struct zsock_sockaddr_in), "IPv4 zaddrlen too short");
		return (struct sockaddr *)zsock_sockaddr_in_to_posix(
			zsock_sin_const(zaddr), (struct sockaddr_in *)buf, out_len);
	case AF_INET6:
		__ASSERT(zaddrlen >= sizeof(struct zsock_sockaddr_in6), "IPv6 zaddrlen too short");
		return (struct sockaddr *)zsock_sockaddr_in6_to_posix(
			zsock_sin6_const(zaddr), (struct sockaddr_in6 *)buf, out_len);
	case AF_UNIX:
		__ASSERT(zaddrlen >= sizeof(struct zsock_sockaddr_un), "UNIX zaddrlen too short");
		return (struct sockaddr *)zsock_sockaddr_un_to_posix(
			(const struct zsock_sockaddr_un *)zaddr, (struct sockaddr_un *)buf,
			out_len);
	default:
		__ASSERT(false, "unsupported Zephyr sockaddr family %u",
			 (unsigned int)zaddr->sa_family);
		return NULL;
	}
}

/** @brief Maximum scatter/gather elements copied on iovec layout mismatch. */
#ifndef POSIX_NET_MSG_MAX_IOV
#define POSIX_NET_MSG_MAX_IOV 16
#endif

/** @brief Stack workspace for msghdr ↔ zsock_msghdr conversion. */
struct posix_zsock_msghdr_buf {
	struct zsock_msghdr msg;
	struct posix_zsock_sockaddr_buf name_buf;
	size_t name_len;
	union {
		struct iovec posix_iov[POSIX_NET_MSG_MAX_IOV];
		struct zsock_iovec zsock_iov[POSIX_NET_MSG_MAX_IOV];
	} iov_copy;
};

static ALWAYS_INLINE bool posix_iovec_layout_eq(void)
{
	return POSIX_NET_STRUCT_LAYOUT_EQ(struct iovec, struct zsock_iovec) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct iovec, struct zsock_iovec, iov_base, iov_base) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct iovec, struct zsock_iovec, iov_len, iov_len);
}

static ALWAYS_INLINE struct zsock_iovec *posix_iovec_to_zsock(struct iovec *iov, size_t iovlen,
							      struct zsock_iovec *buf)
{
	if (iov == NULL || iovlen == 0) {
		return NULL;
	}

	if (posix_iovec_layout_eq()) {
		return (struct zsock_iovec *)iov;
	}

	__ASSERT(iovlen <= POSIX_NET_MSG_MAX_IOV, "msg_iovlen %zu exceeds %d", iovlen,
		 POSIX_NET_MSG_MAX_IOV);

	for (size_t i = 0; i < iovlen; i++) {
		buf[i].iov_base = iov[i].iov_base;
		buf[i].iov_len = iov[i].iov_len;
	}

	return buf;
}

static ALWAYS_INLINE struct iovec *zsock_iovec_to_posix(struct zsock_iovec *iov, size_t iovlen,
							struct iovec *buf)
{
	if (iov == NULL || iovlen == 0) {
		return NULL;
	}

	if (posix_iovec_layout_eq()) {
		return (struct iovec *)iov;
	}

	__ASSERT(iovlen <= POSIX_NET_MSG_MAX_IOV, "msg_iovlen %zu exceeds %d", iovlen,
		 POSIX_NET_MSG_MAX_IOV);

	for (size_t i = 0; i < iovlen; i++) {
		buf[i].iov_base = iov[i].iov_base;
		buf[i].iov_len = iov[i].iov_len;
	}

	return buf;
}

static ALWAYS_INLINE void posix_msghdr_copy_iov_control(const struct msghdr *in,
							struct posix_zsock_msghdr_buf *buf,
							struct zsock_msghdr *out)
{
	out->msg_iov = posix_iovec_to_zsock(in->msg_iov, in->msg_iovlen, buf->iov_copy.zsock_iov);
	out->msg_iovlen = in->msg_iovlen;
	out->msg_control = in->msg_control;
	out->msg_controllen = in->msg_controllen;
	out->msg_flags = in->msg_flags;
}

/**
 * @brief Convert POSIX msghdr to zsock_msghdr for sendmsg() / recvmsg().
 *
 * @p buf must outlive the syscall. Ancillary @c msg_control is passed through
 * unchanged (opaque byte buffer).
 */
static ALWAYS_INLINE struct zsock_msghdr *posix_msghdr_to_zsock(const struct msghdr *in,
								struct posix_zsock_msghdr_buf *buf)
{
	__ASSERT(in != NULL, "in must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");

	memset(buf, 0, sizeof(*buf));
	buf->msg = (struct zsock_msghdr){
		.msg_name = NULL,
		.msg_namelen = 0,
	};
	posix_msghdr_copy_iov_control(in, buf, &buf->msg);

	if (in->msg_name != NULL && in->msg_namelen > 0) {
		buf->msg.msg_name =
			posix_sockaddr_to_zsock(in->msg_name, in->msg_namelen,
						posix_zsock_sa_buf(&buf->name_buf), &buf->name_len);
		buf->msg.msg_namelen = (zsock_socklen_t)buf->name_len;
	}

	return &buf->msg;
}

/**
 * @brief Build zsock_msghdr for recvmsg(); address is received into @p buf.
 */
static ALWAYS_INLINE struct zsock_msghdr *posix_msghdr_for_recv(struct msghdr *in,
								struct posix_zsock_msghdr_buf *buf)
{
	__ASSERT(in != NULL, "in must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");

	memset(buf, 0, sizeof(*buf));
	buf->msg = (struct zsock_msghdr){
		.msg_name = posix_zsock_sa_buf(&buf->name_buf),
		.msg_namelen = sizeof(buf->name_buf),
	};
	posix_msghdr_copy_iov_control(in, buf, &buf->msg);

	return &buf->msg;
}

/**
 * @brief Copy zsock_msghdr results back into a POSIX msghdr after recvmsg().
 */
static ALWAYS_INLINE void zsock_msghdr_to_posix(const struct zsock_msghdr *in, struct msghdr *out,
						struct posix_zsock_msghdr_buf *buf)
{
	__ASSERT(in != NULL, "in must not be NULL");
	__ASSERT(out != NULL, "out must not be NULL");

	out->msg_flags = in->msg_flags;
	out->msg_iov = zsock_iovec_to_posix(in->msg_iov, in->msg_iovlen, buf->iov_copy.posix_iov);
	out->msg_iovlen = in->msg_iovlen;

	if (out->msg_name != NULL && in->msg_name != NULL && in->msg_namelen > 0) {
		size_t out_len = out->msg_namelen;

		zsock_sockaddr_to_posix(in->msg_name, in->msg_namelen, out->msg_name, &out_len);
		out->msg_namelen = (socklen_t)out_len;
	}
}

/** @brief True when POSIX @c struct addrinfo matches @c struct zsock_addrinfo. */
static ALWAYS_INLINE bool posix_addrinfo_layout_eq(void)
{
	return POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_next,
					 ai_next) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_flags,
					 ai_flags) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_family,
					 ai_family) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_socktype,
					 ai_socktype) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_protocol,
					 ai_protocol) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_eflags,
					 ai_eflags) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_addrlen,
					 ai_addrlen) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_addr,
					 ai_addr) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_canonname,
					 ai_canonname) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, _ai_addr,
					 _ai_addr) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, _ai_canonname,
					 _ai_canonname);
}

/** @brief Map POSIX getaddrinfo() hints to zsock_addrinfo (cast or field copy). */
static ALWAYS_INLINE const struct zsock_addrinfo *
posix_addrinfo_hints_to_zsock(const struct addrinfo *hints, struct zsock_addrinfo *out)
{
	__ASSERT(hints != NULL, "hints must not be NULL");
	__ASSERT(out != NULL, "out must not be NULL");

	if (posix_addrinfo_layout_eq()) {
		return (const struct zsock_addrinfo *)hints;
	}

	*out = (struct zsock_addrinfo){
		.ai_flags = hints->ai_flags,
		.ai_family = hints->ai_family,
		.ai_socktype = hints->ai_socktype,
		.ai_protocol = hints->ai_protocol,
		.ai_eflags = hints->ai_eflags,
	};
	return out;
}

static ALWAYS_INLINE struct addrinfo *
posix_addrinfo_node_from_zsock_copy(const struct zsock_addrinfo *zin)
{
	struct addrinfo *out;
	size_t addrlen;

	out = malloc(sizeof(*out));
	if (out == NULL) {
		return NULL;
	}

	*out = (struct addrinfo){
		.ai_next = NULL,
		.ai_flags = zin->ai_flags,
		.ai_family = zin->ai_family,
		.ai_socktype = zin->ai_socktype,
		.ai_protocol = zin->ai_protocol,
		.ai_eflags = zin->ai_eflags,
		.ai_addrlen = 0,
		.ai_addr = NULL,
		.ai_canonname = NULL,
	};

	if (zin->ai_addr != NULL && zin->ai_addrlen > 0) {
		addrlen = sizeof(out->_ai_addr);
		zsock_sockaddr_to_posix(zin->ai_addr, zin->ai_addrlen,
					(struct sockaddr *)&out->_ai_addr, &addrlen);
		out->ai_addr = (struct sockaddr *)&out->_ai_addr;
		out->ai_addrlen = (socklen_t)addrlen;
	}

	if (zin->ai_canonname != NULL) {
		size_t canon_len = strnlen(zin->ai_canonname, sizeof(out->_ai_canonname) - 1);

		memcpy(out->_ai_canonname, zin->ai_canonname, canon_len);
		out->_ai_canonname[canon_len] = '\0';
		out->ai_canonname = out->_ai_canonname;
	}

	return out;
}

static ALWAYS_INLINE void posix_addrinfo_list_free_slow(struct addrinfo *ai)
{
	while (ai != NULL) {
		struct addrinfo *next = ai->ai_next;

		free(ai);
		ai = next;
	}
}

static ALWAYS_INLINE struct addrinfo *
posix_addrinfo_list_from_zsock_slow(struct zsock_addrinfo *zlist)
{
	struct addrinfo *head = NULL;
	struct addrinfo **tail = &head;

	for (struct zsock_addrinfo *z = zlist; z != NULL; z = z->ai_next) {
		struct addrinfo *node = posix_addrinfo_node_from_zsock_copy(z);

		if (node == NULL) {
			posix_addrinfo_list_free_slow(head);
			return NULL;
		}

		*tail = node;
		tail = &node->ai_next;
	}

	return head;
}

/**
 * @brief Convert a zsock_addrinfo list to POSIX addrinfo.
 *
 * Fast path: cast when @c posix_addrinfo_layout_eq(); caller must not
 * @c zsock_freeaddrinfo() the source list (ownership transfers to POSIX
 * @c freeaddrinfo()).
 *
 * Slow path: deep-copy nodes using embedded @c _ai_addr / @c _ai_canonname;
 * caller should @c zsock_freeaddrinfo() the source list after success.
 */
static ALWAYS_INLINE struct addrinfo *posix_addrinfo_list_from_zsock(struct zsock_addrinfo *zlist)
{
	if (posix_addrinfo_layout_eq()) {
		return (struct addrinfo *)zlist;
	}

	return posix_addrinfo_list_from_zsock_slow(zlist);
}

/** @brief Free a POSIX addrinfo list from getaddrinfo(). */
static ALWAYS_INLINE void posix_addrinfo_list_free(struct addrinfo *ai)
{
	if (ai == NULL) {
		return;
	}

	if (posix_addrinfo_layout_eq()) {
		zsock_freeaddrinfo((struct zsock_addrinfo *)ai);
		return;
	}

	posix_addrinfo_list_free_slow(ai);
}

#endif /* ZEPHYR_POSIX_OPTIONS_NETWORKING_ZSOCK_CONVERSION_H_ */
