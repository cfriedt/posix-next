# SPDX-License-Identifier: Apache-2.0

zephyr_library()

# library globals live in the C library partition, so user mode callers can reach them
zephyr_library_app_memory(z_libc_partition)

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared
  ${CMAKE_CURRENT_SOURCE_DIR}/../timers
  ${ZEPHYR_BASE}/kernel/include
  ${ARCH_DIR}/${ARCH}/include
)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
zephyr_library_compile_options_ifdef(CONFIG_XSI -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

if(NOT CONFIG_TC_PROVIDES_POSIX_RW_LOCKS)
  zephyr_library_sources(rwlock.c)
endif()
