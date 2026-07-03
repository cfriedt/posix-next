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
 * 1. @c *_layout_eq() — compile-time check of container @c sizeof and shared-field
 *    offset/size (fields present in only one type are skipped; macro-defined libc
 *    extras are not compared).
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

/**
 * @brief Compare offset and size of one field in a zsock_* vs POSIX struct pair.
 *
 * Fields present in only one type are omitted from the caller's comparison list.
 */
#define POSIX_NET_FIELD_LAYOUT_EQ(native_type, posix_type, native_field, posix_field)            \
	((sizeof(((native_type *)0)->native_field) ==                                               \
	  sizeof(((posix_type *)0)->posix_field)) &&                                               \
	 (offsetof(native_type, native_field) == offsetof(posix_type, posix_field)))

/** @brief True when @p native_type and @p posix_type have identical object size. */
#define POSIX_NET_STRUCT_LAYOUT_EQ(native_type, posix_type)                                      \
	(sizeof(native_type) == sizeof(posix_type))

/*
 * Future conversion sections (same pattern as sockaddr):
 *   zsock_msghdr          ↔ struct msghdr
 *   zsock_iovec           ↔ struct iovec
 *   zsock_cmsghdr         ↔ struct cmsghdr
 *   zsock_addrinfo        ↔ struct addrinfo
 *   zsock_ipv6_mreq       ↔ struct ipv6_mreq
 *   zsock_in_addr         ↔ struct in_addr
 *   zsock_in6_addr        ↔ struct in6_addr
 *
 * pollfd / zvfs_pollfd: POSIX_DEVICE_IO only; poll.h already #define pollfd
 * zvfs_pollfd (same layout as zsock_pollfd). No networking converter expected.
 */

#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>

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

static ALWAYS_INLINE struct zsock_sockaddr *
posix_zsock_sa_buf(struct posix_zsock_sockaddr_buf *buf)
{
	return (struct zsock_sockaddr *)buf->bytes;
}

/* --- sockaddr --- */

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
static ALWAYS_INLINE struct zsock_sockaddr *
posix_sockaddr_to_zsock(const struct sockaddr *addr, size_t addrlen, struct zsock_sockaddr *buf,
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
			(const struct sockaddr_in *)addr, (struct zsock_sockaddr_in *)buf, out_len);
	case AF_INET6:
		__ASSERT(addrlen >= sizeof(struct sockaddr_in6), "IPv6 addrlen too short");
		return (struct zsock_sockaddr *)posix_sockaddr_in6_to_zsock(
			(const struct sockaddr_in6 *)addr, (struct zsock_sockaddr_in6 *)buf, out_len);
	case AF_UNIX:
		__ASSERT(addrlen >= sizeof(struct sockaddr_un), "UNIX addrlen too short");
		return (struct zsock_sockaddr *)posix_sockaddr_un_to_zsock(
			(const struct sockaddr_un *)addr, (struct zsock_sockaddr_un *)buf, out_len);
	default:
		__ASSERT(false, "unsupported POSIX sockaddr family %u", (unsigned)addr->sa_family);
		return NULL;
	}
}

/**
 * @brief Convert a Zephyr zsock_sockaddr to a POSIX sockaddr buffer.
 */
static ALWAYS_INLINE struct sockaddr *
zsock_sockaddr_to_posix(const struct zsock_sockaddr *zaddr, size_t zaddrlen, struct sockaddr *buf,
			size_t *out_len)
{
	__ASSERT(zaddr != NULL, "zaddr must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");
	__ASSERT(out_len != NULL, "out_len must not be NULL");

	switch (zaddr->sa_family) {
	case AF_INET:
		__ASSERT(zaddrlen >= sizeof(struct zsock_sockaddr_in), "IPv4 zaddrlen too short");
		return (struct sockaddr *)zsock_sockaddr_in_to_posix(
			(const struct zsock_sockaddr_in *)zaddr, (struct sockaddr_in *)buf, out_len);
	case AF_INET6:
		__ASSERT(zaddrlen >= sizeof(struct zsock_sockaddr_in6), "IPv6 zaddrlen too short");
		return (struct sockaddr *)zsock_sockaddr_in6_to_posix(
			(const struct zsock_sockaddr_in6 *)zaddr, (struct sockaddr_in6 *)buf, out_len);
	case AF_UNIX:
		__ASSERT(zaddrlen >= sizeof(struct zsock_sockaddr_un), "UNIX zaddrlen too short");
		return (struct sockaddr *)zsock_sockaddr_un_to_posix(
			(const struct zsock_sockaddr_un *)zaddr, (struct sockaddr_un *)buf, out_len);
	default:
		__ASSERT(false, "unsupported Zephyr sockaddr family %u",
			 (unsigned)zaddr->sa_family);
		return NULL;
	}
}

#endif /* ZEPHYR_POSIX_OPTIONS_NETWORKING_ZSOCK_CONVERSION_H_ */
