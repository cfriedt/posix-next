/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
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
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_SOCKLEN_T_DECLARED) || defined(__socklen_t_defined)) || defined(__DOXYGEN__)
/** @brief Type for socket address length values. */
typedef unsigned int socklen_t;
#define _SOCKLEN_T_DECLARED
#define __socklen_t_defined
#endif

#if !(defined(_SA_FAMILY_T_DECLARED) || defined(__sa_family_t_defined)) || defined(__DOXYGEN__)
/** @brief Socket address family type. */
typedef unsigned short sa_family_t;
#define _SA_FAMILY_T_DECLARED
#define __sa_family_t_defined
#endif

#if !(defined(_SOCKADDR_DECLARED) || defined(__sockaddr_defined)) || defined(__DOXYGEN__)
/** @brief Generic socket address. */
struct sockaddr {
	sa_family_t sa_family; /**< Address family. */
	char sa_data[14];        /**< Address payload. */
};
#define _SOCKADDR_DECLARED
#define __sockaddr_defined
#endif

#if !(defined(_SOCKADDR_STORAGE_DECLARED) || defined(__sockaddr_storage_defined)) || \
	defined(__DOXYGEN__)
/** @brief Protocol-independent socket address storage. */
struct sockaddr_storage {
	sa_family_t ss_family; /**< Address family. */
	char __ss_padding[128 - sizeof(sa_family_t)];
};
#define _SOCKADDR_STORAGE_DECLARED
#define __sockaddr_storage_defined
#endif

#include <zephyr/posix/sys/uio.h>

#if !(defined(_MSGHDR_DECLARED) || defined(__msghdr_defined)) || defined(__DOXYGEN__)
/** @brief Message header for sendmsg() and recvmsg(). */
struct msghdr {
	void         *msg_name;       /**< Optional socket address. */
	socklen_t     msg_namelen;    /**< Size of socket address. */
	struct iovec *msg_iov;        /**< Scatter/gather array. */
	size_t        msg_iovlen;     /**< Number of elements in msg_iov. */
	void         *msg_control;    /**< Ancillary data. */
	size_t        msg_controllen; /**< Ancillary data buffer length. */
	int           msg_flags;      /**< Flags on received message. */
};
#define _MSGHDR_DECLARED
#define __msghdr_defined
#endif

#if !(defined(_CMSGHDR_DECLARED) || defined(__cmsghdr_defined)) || defined(__DOXYGEN__)
/** @brief Ancillary data object header. */
struct cmsghdr {
	socklen_t cmsg_len;   /**< Number of bytes, including header. */
	int       cmsg_level; /**< Originating protocol. */
	int       cmsg_type;  /**< Protocol-specific type. */
};
#define _CMSGHDR_DECLARED
#define __cmsghdr_defined
#endif

/** @brief Access rights (file descriptors) in ancillary data. */
#define SCM_RIGHTS 1

#ifndef POSIX_CMSG_ALIGN
#define POSIX_CMSG_ALIGN(n) ((((n) + sizeof(size_t) - 1) / sizeof(size_t)) * sizeof(size_t))
#endif

/** @brief Pointer to ancillary data payload. */
#ifndef CMSG_DATA
#define CMSG_DATA(cmsg) ((unsigned char *)(cmsg) + POSIX_CMSG_ALIGN(sizeof(struct cmsghdr)))
#endif

/** @brief Next ancillary data object in a message. */
#ifndef CMSG_NXTHDR
#define CMSG_NXTHDR(mhdr, cmsg)                                                        \
	(((cmsg) == NULL) ? CMSG_FIRSTHDR(mhdr) :                                        \
	 (((unsigned char *)(cmsg) + POSIX_CMSG_ALIGN((cmsg)->cmsg_len) +                     \
	   POSIX_CMSG_ALIGN(sizeof(struct cmsghdr)) >                                         \
	   (unsigned char *)((mhdr)->msg_control) + (mhdr)->msg_controllen) ?            \
	  NULL :                                                                         \
	  (struct cmsghdr *)((unsigned char *)(cmsg) + POSIX_CMSG_ALIGN((cmsg)->cmsg_len))))
#endif

/** @brief First ancillary data object in a message. */
#ifndef CMSG_FIRSTHDR
#define CMSG_FIRSTHDR(mhdr)                                                            \
	(((mhdr)->msg_controllen >= sizeof(struct cmsghdr) ?                           \
	  (struct cmsghdr *)((mhdr)->msg_control) : NULL))
#endif

#if !(defined(_LINGER_DECLARED) || defined(__linger_defined)) || defined(__DOXYGEN__)
/** @brief Socket linger option structure. */
struct linger {
	int l_onoff;  /**< Indicates whether linger option is enabled. */
	int l_linger; /**< Linger time, in seconds. */
};
#define _LINGER_DECLARED
#define __linger_defined
#endif

/** @brief Length of an ancillary data object including its header. */
#ifndef CMSG_LEN
#define CMSG_LEN(len) (POSIX_CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))
#endif

/** @brief Space occupied by an ancillary data object including padding. */
#ifndef CMSG_SPACE
#define CMSG_SPACE(len) (POSIX_CMSG_ALIGN(sizeof(struct cmsghdr)) + POSIX_CMSG_ALIGN(len))
#endif

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

/** @brief Datagram socket. */
#define SOCK_DGRAM ZSOCK_TYPE_DGRAM
/** @brief Raw socket. */
#define SOCK_RAW ZSOCK_TYPE_RAW
/** @brief Sequenced-packet socket. */
#define SOCK_SEQPACKET ZSOCK_TYPE_SEQPACKET
/** @brief Stream socket. */
#define SOCK_STREAM ZSOCK_TYPE_STREAM

/** @brief Socket-level option level for setsockopt() and getsockopt(). */
#define SOL_SOCKET ZSOCK_SOL_SOCKET

/** @brief Socket is accepting connections. */
#define SO_ACCEPTCONN ZSOCK_SO_ACCEPTCONN
/** @brief Permit sending broadcast datagrams. */
#define SO_BROADCAST ZSOCK_SO_BROADCAST
/** @brief Record debugging information (ignored; for compatibility). */
#define SO_DEBUG ZSOCK_SO_DEBUG
/** @brief Bypass routing when sending (ignored; for compatibility). */
#define SO_DONTROUTE ZSOCK_SO_DONTROUTE
/** @brief Socket error status. */
#define SO_ERROR ZSOCK_SO_ERROR
/** @brief Keep connections alive. */
#define SO_KEEPALIVE ZSOCK_SO_KEEPALIVE
/** @brief Linger on close. */
#define SO_LINGER ZSOCK_SO_LINGER
/** @brief Deliver out-of-band data inline (ignored; for compatibility). */
#define SO_OOBINLINE ZSOCK_SO_OOBINLINE
/** @brief Receive buffer size. */
#define SO_RCVBUF ZSOCK_SO_RCVBUF
/** @brief Receive low watermark. */
#define SO_RCVLOWAT ZSOCK_SO_RCVLOWAT
/** @brief Receive timeout. */
#define SO_RCVTIMEO ZSOCK_SO_RCVTIMEO
/** @brief Allow local address reuse. */
#define SO_REUSEADDR ZSOCK_SO_REUSEADDR
/** @brief Send buffer size. */
#define SO_SNDBUF ZSOCK_SO_SNDBUF
/** @brief Send low watermark. */
#define SO_SNDLOWAT ZSOCK_SO_SNDLOWAT
/** @brief Send timeout. */
#define SO_SNDTIMEO ZSOCK_SO_SNDTIMEO
/** @brief Socket type. */
#define SO_TYPE ZSOCK_SO_TYPE

/** @brief Socket priority. */
#define SO_PRIORITY ZSOCK_SO_PRIORITY
/** @brief Allow multiple sockets to bind the same port. */
#define SO_REUSEPORT ZSOCK_SO_REUSEPORT
/** @brief Bind socket to a network interface. */
#define SO_BINDTODEVICE ZSOCK_SO_BINDTODEVICE
/** @brief Socket protocol. */
#define SO_PROTOCOL ZSOCK_SO_PROTOCOL
/** @brief Socket address family. */
#define SO_DOMAIN ZSOCK_SO_DOMAIN

/** @brief Timestamp TX/RX packets (Zephyr extension). */
#define SO_TIMESTAMPING ZSOCK_SO_TIMESTAMPING
/** @brief Enable SOCKS5 for socket (Zephyr extension). */
#define SO_SOCKS5 ZSOCK_SO_SOCKS5
/** @brief Socket TX time scheduling (Zephyr extension). */
#define SO_TXTIME ZSOCK_SO_TXTIME
/** @brief Ancillary data type for TX time (Zephyr extension). */
#define SCM_TXTIME ZSOCK_SCM_TXTIME

/** @brief Maximum pending connection queue length for listen(). */
#define SOMAXCONN ZSOCK_SOMAXCONN

/** @brief Control data was truncated. */
#define MSG_CTRUNC ZSOCK_MSG_CTRUNC
/** @brief Send without using routing tables. */
#define MSG_DONTROUTE ZSOCK_MSG_DONTROUTE
/** @brief Terminates a record (if supported by the protocol). */
#define MSG_EOR ZSOCK_MSG_EOR
/** @brief Out-of-band data. */
#define MSG_OOB ZSOCK_MSG_OOB
/** @brief No SIGPIPE on send to a disconnected stream socket. */
#define MSG_NOSIGNAL ZSOCK_MSG_NOSIGNAL
/** @brief Peek at incoming data without consuming it. */
#define MSG_PEEK ZSOCK_MSG_PEEK
/** @brief Datagram was truncated. */
#define MSG_TRUNC ZSOCK_MSG_TRUNC
/** @brief Wait for a full request. */
#define MSG_WAITALL ZSOCK_MSG_WAITALL

/** @brief Internet domain sockets for use with IPv4 addresses. */
#define AF_INET ZSOCK_AF_INET
/** @brief Internet domain sockets for use with IPv6 addresses. */
#define AF_INET6 ZSOCK_AF_INET6
/** @brief UNIX domain sockets. */
#define AF_UNIX ZSOCK_AF_UNIX
/** @brief Unspecified address family. */
#define AF_UNSPEC ZSOCK_AF_UNSPEC

/** @brief Disables further receive operations. */
#define SHUT_RD ZSOCK_SHUT_RD
/** @brief Disables further send and receive operations. */
#define SHUT_RDWR ZSOCK_SHUT_RDWR
/** @brief Disables further send operations. */
#define SHUT_WR ZSOCK_SHUT_WR

/** @brief Non-blocking operation. */
#define MSG_DONTWAIT ZSOCK_MSG_DONTWAIT

/**
 * @brief Accept a new connection on a listening socket.
 * @param sock    Listening socket file descriptor.
 * @param addr    Output: address of the connecting peer, or NULL.
 * @param addrlen Input: size of @p addr; output: actual address size.
 * @return New socket file descriptor on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/accept.html
 */
int accept(int sock, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Assign a local address to a socket.
 * @param sock    Socket file descriptor.
 * @param addr    Local address to bind.
 * @param addrlen Size of @p addr in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/bind.html
 */
int bind(int sock, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief Initiate a connection on a socket.
 * @param sock    Socket file descriptor.
 * @param addr    Remote address to connect to.
 * @param addrlen Size of @p addr in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/connect.html
 */
int connect(int sock, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief Get the address of the peer connected to a socket.
 * @param sock    Socket file descriptor.
 * @param addr    Output: peer address.
 * @param addrlen Input: size of @p addr; output: actual size.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpeername.html
 */
int getpeername(int sock, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Get the local address bound to a socket.
 * @param sock    Socket file descriptor.
 * @param addr    Output: local address.
 * @param addrlen Input: size of @p addr; output: actual size.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getsockname.html
 */
int getsockname(int sock, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Get socket options.
 * @param sock    Socket file descriptor.
 * @param level   Protocol level (SOL_SOCKET, IPPROTO_TCP, etc.).
 * @param optname Option name (SO_REUSEADDR, SO_KEEPALIVE, etc.).
 * @param optval  Output: option value.
 * @param optlen  Input: size of @p optval; output: actual size.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getsockopt.html
 */
int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);

/**
 * @brief Mark a socket as passive (ready to accept connections).
 * @param sock    Socket file descriptor.
 * @param backlog Maximum number of pending connections to queue.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/listen.html
 */
int listen(int sock, int backlog);

/**
 * @brief Receive data from a connected socket.
 * @param sock    Socket file descriptor.
 * @param buf     Buffer to receive data into.
 * @param max_len Maximum number of bytes to receive.
 * @param flags   MSG_* flags (MSG_PEEK, MSG_DONTWAIT, etc.).
 * @return Number of bytes received, 0 on connection closed, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/recv.html
 */
ssize_t recv(int sock, void *buf, size_t max_len, int flags);

/**
 * @brief Receive data and the sender's address from a socket.
 * @param sock     Socket file descriptor.
 * @param buf      Buffer to receive data into.
 * @param max_len  Maximum number of bytes to receive.
 * @param flags    MSG_* flags.
 * @param src_addr Output: sender's address, or NULL.
 * @param addrlen  Input: size of @p src_addr; output: actual size.
 * @return Number of bytes received on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/recvfrom.html
 */
ssize_t recvfrom(int sock, void *buf, size_t max_len, int flags, struct sockaddr *src_addr,
		 socklen_t *addrlen);

/**
 * @brief Receive a message (with scatter-gather I/O and ancillary data).
 * @param sock Socket file descriptor.
 * @param msg  Message header specifying I/O vectors and address buffer.
 * @param flags MSG_* flags.
 * @return Number of bytes received on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/recvmsg.html
 */
ssize_t recvmsg(int sock, struct msghdr *msg, int flags);

/**
 * @brief Send data on a connected socket.
 * @param sock Socket file descriptor.
 * @param buf  Data to send.
 * @param len  Number of bytes to send.
 * @param flags MSG_* flags (MSG_DONTWAIT, MSG_NOSIGNAL, etc.).
 * @return Number of bytes sent on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/send.html
 */
ssize_t send(int sock, const void *buf, size_t len, int flags);

/**
 * @brief Send a message (with scatter-gather I/O and ancillary data).
 * @param sock    Socket file descriptor.
 * @param message Message header specifying I/O vectors and destination.
 * @param flags   MSG_* flags.
 * @return Number of bytes sent on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sendmsg.html
 */
ssize_t sendmsg(int sock, const struct msghdr *message, int flags);

/**
 * @brief Send data to a specific destination address.
 * @param sock      Socket file descriptor.
 * @param buf       Data to send.
 * @param len       Number of bytes to send.
 * @param flags     MSG_* flags.
 * @param dest_addr Destination address.
 * @param addrlen   Size of @p dest_addr in bytes.
 * @return Number of bytes sent on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sendto.html
 */
ssize_t sendto(int sock, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr,
	       socklen_t addrlen);

/**
 * @brief Set socket options.
 * @param sock    Socket file descriptor.
 * @param level   Protocol level (SOL_SOCKET, IPPROTO_TCP, etc.).
 * @param optname Option name.
 * @param optval  Pointer to the new option value.
 * @param optlen  Size of @p optval in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setsockopt.html
 */
int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);

/**
 * @brief Shut down part or all of a full-duplex connection.
 * @param sock Socket file descriptor.
 * @param how  SHUT_RD, SHUT_WR, or SHUT_RDWR.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/shutdown.html
 */
int shutdown(int sock, int how);

/**
 * @brief Determine whether a socket is at the out-of-band mark.
 * @param s Socket file descriptor.
 * @return 1 if at the mark, 0 if not, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sockatmark.html
 */
int sockatmark(int s);

/**
 * @brief Create a new socket.
 * @param family Protocol family (AF_INET, AF_INET6, AF_UNIX, etc.).
 * @param type   Socket type (SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, etc.).
 * @param proto  Protocol number (IPPROTO_TCP, IPPROTO_UDP, 0 for default).
 * @return New socket file descriptor on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/socket.html
 */
int socket(int family, int type, int proto);

/**
 * @brief Create a pair of connected sockets.
 * @param family Protocol family (typically AF_UNIX).
 * @param type   Socket type.
 * @param proto  Protocol.
 * @param sv     Output: two-element array receiving the socket descriptors.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/socketpair.html
 */
int socketpair(int family, int type, int proto, int sv[2]);

/** @brief CAN bus address family (Zephyr extension). */
#define AF_CAN ZSOCK_AF_CAN
/** @brief Link-layer packet address family (Zephyr extension). */
#define AF_PACKET ZSOCK_AF_PACKET
/** @brief Network management address family (Zephyr extension). */
#define AF_NET_MGMT ZSOCK_AF_NET_MGMT

/** @brief Unspecified protocol family (common extension). */
#define PF_UNSPEC ZSOCK_PF_UNSPEC
/** @brief IPv4 protocol family (common extension). */
#define PF_INET ZSOCK_PF_INET
/** @brief IPv6 protocol family (common extension). */
#define PF_INET6 ZSOCK_PF_INET6
/** @brief UNIX domain protocol family (common extension). */
#define PF_UNIX ZSOCK_PF_UNIX
/** @brief CAN bus protocol family (Zephyr extension). */
#define PF_CAN ZSOCK_PF_CAN
/** @brief Link-layer packet protocol family (Zephyr extension). */
#define PF_PACKET ZSOCK_PF_PACKET

/** @brief Link-layer socket address (Zephyr extension). */
#define sockaddr_ll zsock_sockaddr_ll

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_SOCKET_H_ */
