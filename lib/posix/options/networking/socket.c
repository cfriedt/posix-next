/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/socket.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/util.h>

#include <zephyr/posix/net/conversion.h>

BUILD_ASSERT(SOCK_STREAM == NET_SOCK_STREAM);
BUILD_ASSERT(SOCK_DGRAM == NET_SOCK_DGRAM);
BUILD_ASSERT(SOCK_RAW == NET_SOCK_RAW);
BUILD_ASSERT(AF_INET == NET_AF_INET);
BUILD_ASSERT(AF_INET6 == NET_AF_INET6);
BUILD_ASSERT(AF_UNIX == NET_AF_UNIX);
BUILD_ASSERT(SOL_SOCKET == ZSOCK_SOL_SOCKET);
BUILD_ASSERT(SO_REUSEADDR == ZSOCK_SO_REUSEADDR);
BUILD_ASSERT(SOMAXCONN == ZSOCK_SOMAXCONN);
BUILD_ASSERT(SHUT_RD == ZSOCK_SHUT_RD);
BUILD_ASSERT(SHUT_WR == ZSOCK_SHUT_WR);
BUILD_ASSERT(SHUT_RDWR == ZSOCK_SHUT_RDWR);
BUILD_ASSERT(MSG_CTRUNC == ZSOCK_MSG_CTRUNC);
BUILD_ASSERT(MSG_PEEK == ZSOCK_MSG_PEEK);
BUILD_ASSERT(MSG_TRUNC == ZSOCK_MSG_TRUNC);
BUILD_ASSERT(MSG_WAITALL == ZSOCK_MSG_WAITALL);

#ifdef CONFIG_MINIMAL_LIBC
BUILD_ASSERT(posix_in_addr_layout_eq());
BUILD_ASSERT(posix_sockaddr_in_layout_eq());
BUILD_ASSERT(posix_sockaddr_in6_layout_eq());
BUILD_ASSERT(posix_sockaddr_un_layout_eq());
BUILD_ASSERT(posix_iovec_layout_eq());
BUILD_ASSERT(posix_msghdr_layout_eq());
BUILD_ASSERT(posix_addrinfo_layout_eq());
#endif

int socket(int family, int type, int proto)
{
	return zsock_socket(family, type, proto);
}
