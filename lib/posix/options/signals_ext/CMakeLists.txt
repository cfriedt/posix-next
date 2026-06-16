# SPDX-License-Identifier: Apache-2.0

set(STRSIGNAL_TABLE_H ${ZEPHYR_BINARY_DIR}/include/generated/posix/strsignal_table.h)

if(NOT CONFIG_TC_PROVIDES_POSIX_SIGNALS_EXT)
  if(CONFIG_POSIX_SIGNALS_EXT AND NOT CONFIG_POSIX_SIGNAL_STRING_DESC_DISABLE)
    add_custom_command(
      OUTPUT ${STRSIGNAL_TABLE_H}
      COMMAND
      ${PYTHON_EXECUTABLE}
      ${ZEPHYR_BASE}/scripts/build/gen_strsignal_table.py
      -i ${ZEPHYR_POSIX_NEXT_MODULE_DIR}/include/zephyr/posix/posix_signal.h
      -o ${STRSIGNAL_TABLE_H}
      DEPENDS ${ZEPHYR_POSIX_NEXT_MODULE_DIR}/include/zephyr/posix/posix_signal.h
    )
  endif()
endif()

zephyr_library()

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared
  ${ZEPHYR_BINARY_DIR}/include/generated
  ${ZEPHYR_BASE}/kernel/include
  ${ARCH_DIR}/${ARCH}/include
)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
zephyr_library_compile_options_ifdef(CONFIG_XSI -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

if(NOT CONFIG_TC_PROVIDES_POSIX_SIGNALS_EXT)
  zephyr_library_sources(strsignal.c)
  zephyr_library_sources_ifndef(CONFIG_POSIX_SIGNAL_STRING_DESC_DISABLE ${STRSIGNAL_TABLE_H})
endif()
