# SPDX-License-Identifier: Apache-2.0

if(CONFIG_NETWORKING OR CONFIG_POSIX_NETWORKING)
  # Expose private header directory for <zephyr/posix/net/conversion.h>
  zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

# zephyr-keep-sorted-start
add_subdirectory_ifdef(CONFIG_EVENTFD eventfd)
add_subdirectory_ifdef(CONFIG_POSIX_C_LANG_SUPPORT_R c_lang_support_r)
add_subdirectory_ifdef(CONFIG_POSIX_C_LIB_EXT c_lib_ext)
add_subdirectory_ifdef(CONFIG_POSIX_SHELL shell)
add_subdirectory_ifdef(CONFIG_POSIX_SYSTEM_INTERFACES options)
# zephyr-keep-sorted-stop
