# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared
  ${ZEPHYR_BASE}/kernel/include
  ${ARCH_DIR}/${ARCH}/include
)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
zephyr_library_compile_options_ifdef(CONFIG_XSI -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

if(NOT CONFIG_TC_PROVIDES_POSIX_ASYNCHRONOUS_IO)
  zephyr_library_sources(
    aio_cancel.c
    aio_error.c
    aio_fsync.c
    aio_read.c
    aio_return.c
    aio_suspend.c
    aio_write.c
    lio_listio.c
  )
endif()
