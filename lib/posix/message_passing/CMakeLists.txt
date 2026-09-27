# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared
  ${CMAKE_CURRENT_SOURCE_DIR}/../timers
  ${ZEPHYR_BASE}/kernel/include
  ${ARCH_DIR}/${ARCH}/include
)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
zephyr_library_compile_options_ifdef(CONFIG_XSI -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

if(NOT CONFIG_TC_PROVIDES_POSIX_MESSAGE_PASSING)
  zephyr_library_sources(
    # zephyr-keep-sorted-start
    mq_close.c
    mq_getattr.c
    mq_notify.c
    mq_open.c
    mq_receive.c
    mq_send.c
    mq_setattr.c
    mq_timedreceive.c
    mq_timedsend.c
    mq_unlink.c
    # zephyr-keep-sorted-stop
  )
endif()
