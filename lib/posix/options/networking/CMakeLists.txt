# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared
  ${ZEPHYR_BASE}/kernel/include
  ${ARCH_DIR}/${ARCH}/include
)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
zephyr_library_compile_options_ifdef(CONFIG_XSI -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

if(NOT CONFIG_TC_PROVIDES_POSIX_NETWORKING)
  zephyr_library_sources(
    accept.c
    bind.c
    connect.c
    freeaddrinfo.c
    gai_strerror.c
    getaddrinfo.c
    getnameinfo.c
    getpeername.c
    gethostname.c
    getsockname.c
    getsockopt.c
    if_indextoname.c
    if_nameindex.c
    if_nametoindex.c
    inet_addr.c
    inet_ntoa.c
    inet_ntop.c
    inet_pton.c
    listen.c
    netdb_ent.c
    recv.c
    recvfrom.c
    recvmsg.c
    send.c
    sendmsg.c
    sendto.c
    setsockopt.c
    shutdown.c
    sockatmark.c
    socket.c
    socketpair.c
  )

  # Share storage with Zephyr's net_in6addr_* rather than defining duplicate
  # POSIX in6addr_any / in6addr_loopback objects (see in6addr_alias.ld).
  zephyr_linker_sources(SECTIONS in6addr_alias.ld)
endif()
