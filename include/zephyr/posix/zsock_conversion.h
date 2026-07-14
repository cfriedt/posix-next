/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr zsock / POSIX network struct conversion functions.
 *
 * Some POSIX implementations use network struct layouts that are identical to Zephyr while
 * others do not.
 *
 * Being identical requires that
 * - the zsock and POSIX structs have the same size
 * - standard struct fields have the same offset and size (although field names may differ)
 * - related constants like @c AF_INET are equal to @c ZSOCK_AF_INET, etc.
 *
 * Conversion functions in this group will first check to see if the layout is identical.
 * If so, then the fast-path is used and a cast version of the input pointer is returned. Otherwise,
 * the fields are copied into the supplied buffer and the a pointer to supplied buffer is returned.
 *
 * The compiler is expected to optimize away the slow path and any unused variables when not
 * required.
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_ZSOCK_CONVERSION_H_
#define ZEPHYR_INCLUDE_POSIX_ZSOCK_CONVERSION_H_

#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>

/** @cond HIDDEN */
#define POSIX_NET_FIELD_LAYOUT_EQ(native_type, posix_type, native_field, posix_field)              \
	((sizeof(((native_type *)0)->native_field) == sizeof(((posix_type *)0)->posix_field)) &&   \
	 (offsetof(native_type, native_field) == offsetof(posix_type, posix_field)))

/** @brief True when @p native_type and @p posix_type have identical object size. */
#define POSIX_NET_STRUCT_LAYOUT_EQ(native_type, posix_type)                                        \
	(sizeof(native_type) == sizeof(posix_type))

struct posix_zsock_sockaddr_buf {
	uint8_t bytes[sizeof(struct sockaddr_storage)];
};

static ALWAYS_INLINE struct zsock_sockaddr *posix_zsock_sa_buf(struct posix_zsock_sockaddr_buf *buf)
{
	return (struct zsock_sockaddr *)buf->bytes;
}
/** @endcond HIDDEN */

/**
 * @brief Compare layout of @ref in_addr and @ref zsock_in_addr.
 *
 * @returns @c true when the layouts are identical, otherwise @c false.
 */
static ALWAYS_INLINE bool posix_in_addr_layout_eq(void)
{
	return POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_in_addr, struct in_addr) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_in_addr, struct in_addr, s_addr, s_addr);
}

/**
 * @brief Compare layout of @ref sockaddr_in and @ref zsock_sockaddr_in.
 *
 * @returns @c true when the layouts are identical, otherwise @c false.
 */
static ALWAYS_INLINE bool posix_sockaddr_in_layout_eq(void)
{
	return (AF_INET == ZSOCK_AF_INET) &&
	       POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_sockaddr_in, struct sockaddr_in) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in, struct sockaddr_in, sin_family,
					 sin_family) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in, struct sockaddr_in, sin_port,
					 sin_port) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in, struct sockaddr_in, sin_addr,
					 sin_addr);
}

/**
 * @brief Compare layout of @ref sockaddr_in6 and @ref zsock_sockaddr_in6.
 *
 * @returns @c true when the layouts are identical, otherwise @c false.
 */
static ALWAYS_INLINE bool posix_sockaddr_in6_layout_eq(void)
{
	return (AF_INET6 == ZSOCK_AF_INET6) &&
	       POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6,
					 sin6_family, sin6_family) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6, sin6_port,
					 sin6_port) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6, sin6_addr,
					 sin6_addr) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_in6, struct sockaddr_in6,
					 sin6_scope_id, sin6_scope_id);
}

/**
 * @brief Compare layout of @ref sockaddr_un and @ref zsock_sockaddr_un.
 *
 * @returns @c true if layouts match, @c false otherwise.
 */
static ALWAYS_INLINE bool posix_sockaddr_un_layout_eq(void)
{
	return POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_sockaddr_un, struct sockaddr_un) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_un, struct sockaddr_un, sun_family,
					 sun_family) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_sockaddr_un, struct sockaddr_un, sun_path,
					 sun_path);
}

/**
 * @brief Convert @ref sockaddr_in to @ref zsock_sockaddr_in.
 *
 * @param in The @ref sockaddr_in to convert.
 * @param[out] buf The buffer to store the converted @ref zsock_sockaddr_in.
 * @param[inout] out_len The length of the converted @ref zsock_sockaddr_in.
 *
 * @returns A pointer to the converted @ref zsock_sockaddr_in.
 */
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

/**
 * @brief Convert @ref zsock_sockaddr_in to @ref sockaddr_in.
 *
 * @param in The @ref zsock_sockaddr_in to convert.
 * @param[out] buf The buffer to store the converted @ref sockaddr_in.
 * @param[inout] out_len The length of the converted @ref sockaddr_in.
 *
 * @returns A pointer to the converted @ref sockaddr_in.
 */
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

/**
 * @brief Convert @ref sockaddr_in6 to @ref zsock_sockaddr_in6.
 *
 * @param in The @ref sockaddr_in6 to convert.
 * @param[out] buf The buffer to store the converted @ref zsock_sockaddr_in6.
 * @param[inout] out_len The length of the converted @ref zsock_sockaddr_in6.
 *
 * @returns A pointer to the converted @ref zsock_sockaddr_in6.
 */
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

/**
 * @brief Convert @ref zsock_sockaddr_in6 to @ref sockaddr_in6.
 *
 * @param in The @ref zsock_sockaddr_in6 to convert.
 * @param[out] buf The buffer to store the converted @ref sockaddr_in6.
 * @param[inout] out_len The length of the converted @ref sockaddr_in6.
 *
 * @returns A pointer to the converted @ref sockaddr_in6.
 */
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
		.sin6_scope_id = in->sin6_scope_id,
	};
	memcpy(&out->sin6_addr, &in->sin6_addr, sizeof(out->sin6_addr));
	*out_len = sizeof(struct sockaddr_in6);
	return out;
}

/**
 * @brief Convert @ref sockaddr_un to @ref zsock_sockaddr_un.
 *
 * @param in The @ref sockaddr_un to convert.
 * @param[out] buf The buffer to store the converted @ref zsock_sockaddr_un.
 * @param[inout] out_len The length of the converted @ref zsock_sockaddr_un.
 *
 * @returns A pointer to the converted @ref zsock_sockaddr_un.
 */
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

/**
 * @brief Convert @ref zsock_sockaddr_un to @ref sockaddr_un.
 *
 * @param in The @ref zsock_sockaddr_un to convert.
 * @param[out] buf The buffer to store the converted @ref sockaddr_un.
 * @param[inout] out_len The length of the converted @ref sockaddr_un.
 *
 * @returns A pointer to the converted @ref sockaddr_un.
 */
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
 * @brief Convert @ref sockaddr to @ref zsock_sockaddr.
 *
 * If the layout of @ref sockaddr and @ref zsock_sockaddr are identical,
 * the fast-path is used and a cast version of @p addr is returned. Otherwise,
 * the fields are copied into the supplied buffer and the a pointer to supplied buffer is returned.
 *
 * The compiler is expected to optimize away the slow path and any unused variables when not
 * required.
 *
 * @param addr The @ref sockaddr to convert.
 * @param addrlen The length of the @ref sockaddr.
 * @param[out] buf The buffer to store the converted @ref zsock_sockaddr.
 * @param[inout] out_len The length of the converted @ref zsock_sockaddr.
 *
 * @returns A pointer to the converted @ref zsock_sockaddr.
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
		__ASSERT(addrlen >= sizeof(struct zsock_sockaddr_in), "IPv4 addrlen too short");
		return (struct zsock_sockaddr *)posix_sockaddr_in_to_zsock(
			(const struct sockaddr_in *)addr, (struct zsock_sockaddr_in *)buf, out_len);
	case AF_INET6:
		__ASSERT(addrlen >= sizeof(struct zsock_sockaddr_in6), "IPv6 addrlen too short");
		return (struct zsock_sockaddr *)posix_sockaddr_in6_to_zsock(
			(const struct sockaddr_in6 *)addr, (struct zsock_sockaddr_in6 *)buf,
			out_len);
	case AF_UNIX:
		__ASSERT(addrlen >= sizeof(struct zsock_sockaddr_un), "UNIX addrlen too short");
		return (struct zsock_sockaddr *)posix_sockaddr_un_to_zsock(
			(const struct sockaddr_un *)addr, (struct zsock_sockaddr_un *)buf, out_len);
	default:
		/*
		 * Non-POSIX address families (e.g. AF_CAN, AF_PACKET) use
		 * Zephyr-native structs that require no conversion. Pass
		 * the address through unchanged.
		 */
		*out_len = addrlen;
		return (struct zsock_sockaddr *)addr;
	}
}

/**
 * @brief Convert @ref zsock_sockaddr to @ref sockaddr.
 *
 * If the layout of @ref zsock_sockaddr and @ref sockaddr are identical,
 * the fast-path is used and a cast version of @p zaddr is returned. Otherwise,
 * the fields are copied into the supplied buffer and the a pointer to supplied buffer is returned.
 *
 * The compiler is expected to optimize away the slow path and any unused variables when not
 * required.
 *
 * @param zaddr The @ref zsock_sockaddr to convert.
 * @param zaddrlen The length of the @ref zsock_sockaddr.
 * @param[out] buf The buffer to store the converted @ref sockaddr.
 * @param[inout] out_len The length of the converted @ref sockaddr.
 *
 * @returns A pointer to the converted @ref sockaddr.
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
			(const struct zsock_sockaddr_in *)zaddr, (struct sockaddr_in *)buf,
			out_len);
	case AF_INET6:
		__ASSERT(zaddrlen >= sizeof(struct zsock_sockaddr_in6), "IPv6 zaddrlen too short");
		return (struct sockaddr *)zsock_sockaddr_in6_to_posix(
			(const struct zsock_sockaddr_in6 *)zaddr, (struct sockaddr_in6 *)buf,
			out_len);
	case AF_UNIX:
		__ASSERT(zaddrlen >= sizeof(struct zsock_sockaddr_un), "UNIX zaddrlen too short");
		return (struct sockaddr *)zsock_sockaddr_un_to_posix(
			(const struct zsock_sockaddr_un *)zaddr, (struct sockaddr_un *)buf,
			out_len);
	default:
		/*
		 * Non-POSIX address families (e.g. AF_CAN, AF_PACKET) use
		 * Zephyr-native structs that require no conversion. Pass
		 * the address through unchanged.
		 */
		*out_len = zaddrlen;
		return (struct sockaddr *)zaddr;
	}
}

/**
 * @brief Convert a pointer to @ref sockaddr_in.
 *
 * @param addr The pointer to convert.
 *
 * @returns A pointer to the converted @ref sockaddr_in.
 */
static ALWAYS_INLINE struct sockaddr_in *posix_sin(void *addr)
{
	return (struct sockaddr_in *)addr;
}

/**
 * @brief Convert a pointer to @ref sockaddr_in6.
 *
 * @param addr The pointer to convert.
 *
 * @returns A pointer to the converted @ref sockaddr_in6.
 */
static ALWAYS_INLINE struct sockaddr_in6 *posix_sin6(void *addr)
{
	return (struct sockaddr_in6 *)addr;
}

/**
 * @brief Compare layout of @ref iovec and @ref zsock_iovec.
 *
 * @returns @c true when the layouts are identical, otherwise @c false.
 */
static ALWAYS_INLINE bool posix_iovec_layout_eq(void)
{
	return POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_iovec, struct iovec) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_iovec, struct iovec, iov_base, iov_base) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_iovec, struct iovec, iov_len, iov_len);
}

/**
 * @brief Compare layout of @ref msghdr and @ref zsock_msghdr.
 *
 * Includes an @ref iovec layout check via @ref posix_iovec_layout_eq.
 *
 * @note @c msg_iovlen is @c int in POSIX but @c size_t in Zephyr; on 64-bit targets
 *       these differ in size so this function returns @c false.  Similarly,
 *       @c msg_controllen is @c socklen_t (32-bit) in POSIX but @c size_t (64-bit
 *       on 64-bit targets) in Zephyr.  The fast path therefore only triggers on
 *       32-bit platforms where the sizes coincide.
 *
 * @returns @c true when the layouts are identical, otherwise @c false.
 */
static ALWAYS_INLINE bool posix_msghdr_layout_eq(void)
{
	return posix_iovec_layout_eq() &&
	       POSIX_NET_STRUCT_LAYOUT_EQ(struct zsock_msghdr, struct msghdr) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_msghdr, struct msghdr, msg_name, msg_name) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_msghdr, struct msghdr, msg_namelen,
					 msg_namelen) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_msghdr, struct msghdr, msg_iov, msg_iov) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_msghdr, struct msghdr, msg_iovlen,
					 msg_iovlen) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_msghdr, struct msghdr, msg_control,
					 msg_control) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_msghdr, struct msghdr, msg_controllen,
					 msg_controllen) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_msghdr, struct msghdr, msg_flags, msg_flags);
}

/**
 * @brief Convert a POSIX @ref msghdr to a @ref zsock_msghdr.
 *
 * Fast path: when @ref posix_msghdr_layout_eq, @ref posix_sockaddr_in_layout_eq, and
 * @ref posix_sockaddr_in6_layout_eq all hold, returns a direct cast of @p msg so that
 * @c msg_name is passed through without a copy.
 *
 * Slow path: populates @p buf field-by-field.  @c msg_name is copied as a raw
 * @c void* — the caller must overwrite @c buf->msg_name with a properly converted
 * zsock address when the two sockaddr layouts differ.
 *
 * @param msg   POSIX msghdr source; must not be @c NULL.
 * @param[out] buf  Scratch buffer for the slow path; must not be @c NULL.
 * @returns Pointer to the populated @ref zsock_msghdr.
 */
static ALWAYS_INLINE struct zsock_msghdr *posix_msghdr_to_zsock(const struct msghdr *msg,
								 struct zsock_msghdr *buf)
{
	__ASSERT(msg != NULL, "msg must not be NULL");
	__ASSERT(buf != NULL, "buf must not be NULL");

	if (posix_msghdr_layout_eq() && posix_sockaddr_in_layout_eq() &&
	    posix_sockaddr_in6_layout_eq()) {
		return (struct zsock_msghdr *)msg;
	}

	*buf = (struct zsock_msghdr){
		.msg_name = msg->msg_name,
		.msg_namelen = msg->msg_namelen,
		.msg_iov = (struct zsock_iovec *)msg->msg_iov,
		.msg_iovlen = (size_t)msg->msg_iovlen,
		.msg_control = msg->msg_control,
		.msg_controllen = (size_t)msg->msg_controllen,
		.msg_flags = msg->msg_flags,
	};
	return buf;
}

/**
 * @brief Copy @ref zsock_msghdr output fields back into a POSIX @ref msghdr after recvmsg.
 *
 * Updates @c msg_namelen, @c msg_controllen, and @c msg_flags.  @c msg_name contents
 * are written in-place by @c zsock_recvmsg; @c msg_iov data is written into the
 * caller's scatter/gather buffers (also in-place).
 *
 * Fast path: when @p zmsg aliases @p msg (i.e. a cast was used in @ref posix_msghdr_to_zsock),
 * all fields are already up to date and nothing is copied.
 *
 * @param zmsg  Source @ref zsock_msghdr; must not be @c NULL.
 * @param[out] msg  Destination POSIX @ref msghdr; must not be @c NULL.
 */
static ALWAYS_INLINE void zsock_msghdr_to_posix(const struct zsock_msghdr *zmsg,
						 struct msghdr *msg)
{
	__ASSERT(zmsg != NULL, "zmsg must not be NULL");
	__ASSERT(msg != NULL, "msg must not be NULL");

	if ((const void *)zmsg == (const void *)msg) {
		return;
	}

	msg->msg_namelen = (socklen_t)zmsg->msg_namelen;
	msg->msg_controllen = (socklen_t)zmsg->msg_controllen;
	msg->msg_flags = zmsg->msg_flags;
}

/**
 * @brief Compare layout of @ref addrinfo and @ref zsock_addrinfo.
 *
 * Returns @c true when a plain cast between the two types is safe, i.e. every
 * standard field occupies the same offset and has the same size.
 *
 * @returns @c true when the layouts are identical, otherwise @c false.
 */
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
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_addrlen,
					 ai_addrlen) &&
	       POSIX_NET_FIELD_LAYOUT_EQ(struct zsock_addrinfo, struct addrinfo, ai_canonname,
					 ai_canonname);
}

/**
 * @brief Convert a POSIX @ref addrinfo hints struct to a @ref zsock_addrinfo hints struct.
 *
 * Only the lookup-relevant hint fields (@c ai_flags, @c ai_family, @c ai_socktype,
 * @c ai_protocol) are copied; all other fields (addr, canonname, next) are zeroed.
 *
 * When the two layouts are identical (fast path) the original pointer is returned
 * as a const cast and @p buf is not written.
 *
 * @param hints  POSIX hints to convert; may be @c NULL (returns @c NULL).
 * @param[out] buf  Scratch buffer to use on the slow path.
 *
 * @returns Pointer to a valid @ref zsock_addrinfo hints struct, or @c NULL.
 */
static ALWAYS_INLINE const struct zsock_addrinfo *
posix_addrinfo_hints_to_zsock(const struct addrinfo *hints, struct zsock_addrinfo *buf)
{
	if (hints == NULL) {
		return NULL;
	}

	if (posix_addrinfo_layout_eq()) {
		return (const struct zsock_addrinfo *)hints;
	}

	*buf = (struct zsock_addrinfo){
		.ai_flags    = hints->ai_flags,
		.ai_family   = hints->ai_family,
		.ai_socktype = hints->ai_socktype,
		.ai_protocol = hints->ai_protocol,
	};
	return buf;
}

/**
 * @brief Copy fields from a @ref zsock_addrinfo result into a POSIX @ref addrinfo.
 *
 * Used on the slow path to populate a POSIX @c addrinfo wrapper that overlays an
 * existing @ref zsock_addrinfo node.  Only the fields visible to POSIX callers are
 * copied; the @c ai_addr and @c ai_next pointers are patched up by the caller.
 *
 * @param src   Source @ref zsock_addrinfo node.
 * @param[out] dst  Destination @ref addrinfo to populate.
 */
static ALWAYS_INLINE void zsock_addrinfo_to_posix_fields(const struct zsock_addrinfo *src,
							 struct addrinfo *dst)
{
	dst->ai_flags    = src->ai_flags;
	dst->ai_family   = src->ai_family;
	dst->ai_socktype = src->ai_socktype;
	dst->ai_protocol = src->ai_protocol;
	dst->ai_addrlen  = src->ai_addrlen;
	dst->ai_addr     = (struct sockaddr *)src->ai_addr;
	dst->ai_canonname = src->ai_canonname;
}

#endif /* ZEPHYR_INCLUDE_POSIX_ZSOCK_CONVERSION_H_ */
