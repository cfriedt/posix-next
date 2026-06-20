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

if(NOT CONFIG_TC_PROVIDES_POSIX_SYSTEM_DATABASE_R)
  zephyr_library_sources(
    system_database_priv.c
    system_database_r.c
  )
endif()
