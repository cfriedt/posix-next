/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX socket API (<sys/socket.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_socket.h.html">
 *      POSIX.1-2017 &lt;sys/socket.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_SOCKET_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_SOCKET_H_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <zephyr/posix/sys/time.h>
#include <zephyr/net/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_SOCKLEN_T_DECLARED) || defined(__socklen_t_defined)) || defined(__DOXYGEN__)
/** @brief Unsigned integer type of width at least 32 bits for socket address lengths. */
typedef uint32_t socklen_t;
#define _SOCKLEN_T_DECLARED
#define __socklen_t_defined
#endif

#if !(defined(_SA_FAMILY_T_DECLARED) || defined(__sa_family_t_defined)) || defined(__DOXYGEN__)
/** @brief Unsigned integer type for socket address families. */
typedef uint16_t sa_family_t;
#define _SA_FAMILY_T_DECLARED
#define __sa_family_t_defined
#endif

#if !(defined(_SOCKADDR_DECLARED) || defined(__sockaddr_defined)) || defined(__DOXYGEN__)
/** @brief Generic socket address. */
struct sockaddr {
	/** @brief Address family. */
	sa_family_t sa_family;
	/** @brief Socket address (variable-length data). */
	char sa_data[];
};
#define _SOCKADDR_DECLARED
#define __sockaddr_defined
#endif

#if !(defined(_SOCKADDR_STORAGE_DECLARED) || defined(__sockaddr_storage_defined)) ||               \
	defined(__DOXYGEN__)
/** @brief Storage large enough for any supported socket address. */
struct sockaddr_storage {
	/** @brief Address family. */
	sa_family_t ss_family;
	/** @brief Padding to implementation-defined maximum size. */
	char __ss_padding[128 - sizeof(sa_family_t)];
};
#define _SOCKADDR_STORAGE_DECLARED
#define __sockaddr_storage_defined
#endif

#if !defined(_IOVEC_DECLARED) && !defined(__iovec_defined)
struct iovec {
	void *iov_base;
	size_t iov_len;
};
#define _IOVEC_DECLARED
#define __iovec_defined
#endif

#if !(defined(_MSGHDR_DECLARED) || defined(__msghdr_defined)) || defined(__DOXYGEN__)
/** @brief Message header for recvmsg() and sendmsg(). */
struct msghdr {
	/** @brief Optional socket address. */
	void *msg_name;
	/** @brief Size of @p msg_name. */
	socklen_t msg_namelen;
	/** @brief Scatter/gather array. */
	struct iovec *msg_iov;
	/** @brief Number of elements in @p msg_iov. */
	int msg_iovlen;
	/** @brief Ancillary data. */
	void *msg_control;
	/** @brief Ancillary data buffer length. */
	socklen_t msg_controllen;
	/** @brief Flags on received message. */
	int msg_flags;
};
#define _MSGHDR_DECLARED
#define __msghdr_defined
#endif

#if !(defined(_CMSGHDR_DECLARED) || defined(__cmsghdr_defined)) || defined(__DOXYGEN__)
/** @brief Ancillary data object header. */
struct cmsghdr {
	/** @brief Data byte count, including the cmsghdr. */
	socklen_t cmsg_len;
	/** @brief Originating protocol. */
	int cmsg_level;
	/** @brief Protocol-specific type. */
	int cmsg_type;
};
#define _CMSGHDR_DECLARED
#define __cmsghdr_defined
#endif

/** @brief Access rights to be sent or received in ancillary data. */
#define SCM_RIGHTS 0x01

#if !defined(_CMSG_ALIGN)
#define _CMSG_ALIGN(len) (((len) + sizeof(long) - 1) & ~(sizeof(long) - 1U))
#endif

/** @brief Pointer to ancillary data following a cmsghdr. */
#define CMSG_DATA(cmsg) ((unsigned char *)(void *)(cmsg) + _CMSG_ALIGN(sizeof(struct cmsghdr)))

/** @brief Pointer to the next cmsghdr in ancillary data, or NULL. */
#define CMSG_NXTHDR(mhdr, cmsg)                                                                    \
	((cmsg) == NULL ? CMSG_FIRSTHDR(mhdr)                                                      \
			: (((unsigned char *)(cmsg) + _CMSG_ALIGN((cmsg)->cmsg_len) +              \
			    _CMSG_ALIGN(sizeof(struct cmsghdr))) >                                 \
					   ((unsigned char *)(mhdr)->msg_control +                 \
					    (mhdr)->msg_controllen)                                \
				   ? NULL                                                          \
				   : (struct cmsghdr *)((unsigned char *)(cmsg) +                  \
							_CMSG_ALIGN((cmsg)->cmsg_len))))

/** @brief Pointer to the first cmsghdr in ancillary data, or NULL. */
#define CMSG_FIRSTHDR(mhdr)                                                                        \
	((mhdr)->msg_controllen >= sizeof(struct cmsghdr)                                          \
		 ? (struct cmsghdr *)(void *)(mhdr)->msg_control                                   \
		 : (struct cmsghdr *)(void *)0)

/** @brief Length of a cmsghdr plus associated data. */
#define CMSG_LEN(length) (_CMSG_ALIGN(sizeof(struct cmsghdr)) + (length))

/** @brief Space required for a cmsghdr plus associated data, with alignment. */
#define CMSG_SPACE(length) (_CMSG_ALIGN(sizeof(struct cmsghdr)) + _CMSG_ALIGN(length))

#if !(defined(_LINGER_DECLARED) || defined(__linger_defined)) || defined(__DOXYGEN__)
/** @brief Socket linger option. */
struct linger {
	/** @brief Non-zero if linger is enabled. */
	int l_onoff;
	/** @brief Linger time in seconds. */
	int l_linger;
};
#define _LINGER_DECLARED
#define __linger_defined
#endif

/** @brief Stream socket type. */
#define SOCK_STREAM ZSOCK_SOCK_STREAM
/** @brief Datagram socket type. */
#define SOCK_DGRAM  ZSOCK_SOCK_DGRAM
/** @brief Raw socket type. */
#define SOCK_RAW    ZSOCK_SOCK_RAW

/** @brief Options accessed at the socket level. */
#define SOL_SOCKET ZSOCK_SOL_SOCKET

/** @brief Socket is accepting connections. */
#define SO_ACCEPTCONN ZSOCK_SO_ACCEPTCONN
/** @brief Transmission of broadcast messages is supported. */
#define SO_BROADCAST  ZSOCK_SO_BROADCAST
/** @brief Debugging information is being recorded. */
#define SO_DEBUG      ZSOCK_SO_DEBUG
/** @brief Bypass normal routing. */
#define SO_DONTROUTE  ZSOCK_SO_DONTROUTE
/** @brief Socket error status. */
#define SO_ERROR      ZSOCK_SO_ERROR
/** @brief Connections are kept alive with periodic messages. */
#define SO_KEEPALIVE  ZSOCK_SO_KEEPALIVE
/** @brief Socket lingers on close. */
#define SO_LINGER     ZSOCK_SO_LINGER
/** @brief Out-of-band data is transmitted in line. */
#define SO_OOBINLINE  ZSOCK_SO_OOBINLINE
/** @brief Receive buffer size. */
#define SO_RCVBUF     ZSOCK_SO_RCVBUF
/** @brief Receive low water mark. */
#define SO_RCVLOWAT   ZSOCK_SO_RCVLOWAT
/** @brief Receive timeout. */
#define SO_RCVTIMEO   ZSOCK_SO_RCVTIMEO
/** @brief Reuse of local addresses is supported. */
#define SO_REUSEADDR  ZSOCK_SO_REUSEADDR
/** @brief Send buffer size. */
#define SO_SNDBUF     ZSOCK_SO_SNDBUF
/** @brief Send low water mark. */
#define SO_SNDLOWAT   ZSOCK_SO_SNDLOWAT
/** @brief Send timeout. */
#define SO_SNDTIMEO   ZSOCK_SO_SNDTIMEO
/** @brief Socket type. */
#define SO_TYPE       ZSOCK_SO_TYPE

/** @brief Maximum backlog queue length for listen(). */
#define SOMAXCONN ZSOCK_SOMAXCONN

/** @brief Control data truncated. */
#define MSG_CTRUNC    ZSOCK_MSG_CTRUNC
/** @brief Send without using routing tables. */
#define MSG_DONTROUTE 0x04
/** @brief Terminates a record (if supported by the protocol). */
#define MSG_EOR       0x80
/** @brief Out-of-band data. */
#define MSG_OOB       0x01
/** @brief Do not generate SIGPIPE on stream socket send failures. */
#define MSG_NOSIGNAL  0x4000
/** @brief Leave received data in queue. */
#define MSG_PEEK      ZSOCK_MSG_PEEK
/** @brief Normal data truncated. */
#define MSG_TRUNC     ZSOCK_MSG_TRUNC
/** @brief Attempt to fill the read buffer. */
#define MSG_WAITALL   ZSOCK_MSG_WAITALL

/** @brief Unspecified address family. */
#define AF_UNSPEC ZSOCK_AF_UNSPEC
/** @brief IPv4 address family. */
#define AF_INET   ZSOCK_AF_INET
/** @brief IPv6 address family. */
#define AF_INET6  ZSOCK_AF_INET6
/** @brief UNIX domain address family. */
#define AF_UNIX   ZSOCK_AF_UNIX
/** @brief Unspecified protocol family. */
#define PF_UNSPEC ZSOCK_PF_UNSPEC
/** @brief IPv4 protocol family. */
#define PF_INET   ZSOCK_PF_INET
/** @brief IPv6 protocol family. */
#define PF_INET6  ZSOCK_PF_INET6
/** @brief UNIX domain protocol family. */
#define PF_UNIX   ZSOCK_PF_UNIX

/** @brief Disable further receive operations. */
#define SHUT_RD   ZSOCK_SHUT_RD
/** @brief Disable further send and receive operations. */
#define SHUT_RDWR ZSOCK_SHUT_RDWR
/** @brief Disable further send operations. */
#define SHUT_WR   ZSOCK_SHUT_WR

/**
 * @brief Accept a new connection on a listening socket.
 *
 * @param sock    Listening socket file descriptor.
 * @param addr    Output: address of the connecting peer, or NULL.
 * @param addrlen Input: size of @p addr; output: actual address size.
 * @return New socket file descriptor on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/accept.html
 */
int accept(int sock, struct sockaddr *restrict addr, socklen_t *restrict addrlen);

/**
 * @brief Assign a local address to a socket.
 *
 * @param sock    Socket file descriptor.
 * @param addr    Local address to bind.
 * @param addrlen Size of @p addr in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/bind.html
 */
int bind(int sock, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief Initiate a connection on a socket.
 *
 * @param sock    Socket file descriptor.
 * @param addr    Remote address to connect to.
 * @param addrlen Size of @p addr in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/connect.html
 */
int connect(int sock, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief Get the address of the peer connected to a socket.
 *
 * @param sock    Socket file descriptor.
 * @param addr    Output: peer address.
 * @param addrlen Input: size of @p addr; output: actual size.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpeername.html
 */
int getpeername(int sock, struct sockaddr *restrict addr, socklen_t *restrict addrlen);

/**
 * @brief Get the local address bound to a socket.
 *
 * @param sock    Socket file descriptor.
 * @param addr    Output: local address.
 * @param addrlen Input: size of @p addr; output: actual size.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getsockname.html
 */
int getsockname(int sock, struct sockaddr *restrict addr, socklen_t *restrict addrlen);

/**
 * @brief Get socket options.
 *
 * @param sock    Socket file descriptor.
 * @param level   Protocol level (@c SOL_SOCKET, @c IPPROTO_TCP, etc.).
 * @param optname Option name.
 * @param optval  Output: option value.
 * @param optlen  Input: size of @p optval; output: actual size.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getsockopt.html
 */
int getsockopt(int sock, int level, int optname, void *restrict optval,
	       socklen_t *restrict optlen);

/**
 * @brief Mark a socket as passive (ready to accept connections).
 *
 * @param sock    Socket file descriptor.
 * @param backlog Maximum number of pending connections to queue.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/listen.html
 */
int listen(int sock, int backlog);

/**
 * @brief Receive data from a connected socket.
 *
 * @param sock    Socket file descriptor.
 * @param buf     Buffer to receive data into.
 * @param max_len Maximum number of bytes to receive.
 * @param flags   @c MSG_* flags.
 * @return Number of bytes received, 0 on connection closed, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/recv.html
 */
ssize_t recv(int sock, void *buf, size_t max_len, int flags);

/**
 * @brief Receive data and the sender's address from a socket.
 *
 * @param sock     Socket file descriptor.
 * @param buf      Buffer to receive data into.
 * @param max_len  Maximum number of bytes to receive.
 * @param flags    @c MSG_* flags.
 * @param src_addr Output: sender's address, or NULL.
 * @param addrlen  Input: size of @p src_addr; output: actual size.
 * @return Number of bytes received on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/recvfrom.html
 */
ssize_t recvfrom(int sock, void *restrict buf, size_t max_len, int flags,
		 struct sockaddr *restrict src_addr, socklen_t *restrict addrlen);

/**
 * @brief Receive a message (with scatter/gather I/O and ancillary data).
 *
 * @param sock Socket file descriptor.
 * @param msg  Message header specifying I/O vectors and address buffer.
 * @param flags @c MSG_* flags.
 * @return Number of bytes received on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/recvmsg.html
 */
ssize_t recvmsg(int sock, struct msghdr *msg, int flags);

/**
 * @brief Send data on a connected socket.
 *
 * @param sock Socket file descriptor.
 * @param buf  Data to send.
 * @param len  Number of bytes to send.
 * @param flags @c MSG_* flags.
 * @return Number of bytes sent on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/send.html
 */
ssize_t send(int sock, const void *buf, size_t len, int flags);

/**
 * @brief Send a message (with scatter/gather I/O and ancillary data).
 *
 * @param sock    Socket file descriptor.
 * @param message Message header specifying I/O vectors and destination.
 * @param flags   @c MSG_* flags.
 * @return Number of bytes sent on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sendmsg.html
 */
ssize_t sendmsg(int sock, const struct msghdr *message, int flags);

/**
 * @brief Send data to a specific destination address.
 *
 * @param sock      Socket file descriptor.
 * @param buf       Data to send.
 * @param len       Number of bytes to send.
 * @param flags     @c MSG_* flags.
 * @param dest_addr Destination address.
 * @param addrlen   Size of @p dest_addr in bytes.
 * @return Number of bytes sent on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sendto.html
 */
ssize_t sendto(int sock, const void *buf, size_t len, int flags,
	       const struct sockaddr *dest_addr, socklen_t addrlen);

/**
 * @brief Set socket options.
 *
 * @param sock    Socket file descriptor.
 * @param level   Protocol level (@c SOL_SOCKET, @c IPPROTO_TCP, etc.).
 * @param optname Option name.
 * @param optval  Pointer to the new option value.
 * @param optlen  Size of @p optval in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setsockopt.html
 */
int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);

/**
 * @brief Shut down part or all of a full-duplex connection.
 *
 * @param sock Socket file descriptor.
 * @param how  @c SHUT_RD, @c SHUT_WR, or @c SHUT_RDWR.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/shutdown.html
 */
int shutdown(int sock, int how);

/**
 * @brief Determine whether a socket is at the out-of-band mark.
 *
 * @param s Socket file descriptor.
 * @return 1 if at the mark, 0 if not, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sockatmark.html
 */
int sockatmark(int s);

/**
 * @brief Create a new socket.
 *
 * @param family Protocol family (@c AF_INET, @c AF_INET6, @c AF_UNIX, etc.).
 * @param type   Socket type (@c SOCK_STREAM, @c SOCK_DGRAM, @c SOCK_RAW, etc.).
 * @param proto  Protocol number (@c IPPROTO_TCP, @c IPPROTO_UDP, or 0 for default).
 * @return New socket file descriptor on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/socket.html
 */
int socket(int family, int type, int proto);

/**
 * @brief Create a pair of connected sockets.
 *
 * @param family Protocol family (typically @c AF_UNIX).
 * @param type   Socket type.
 * @param proto  Protocol.
 * @param sv     Output: two-element array receiving the socket descriptors.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/socketpair.html
 */
int socketpair(int family, int type, int proto, int sv[2]);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_SOCKET_H_ */
