# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_KCL_LINUX linux)

# the kernel's hooks when no layer loads images
if(CONFIG_KCL AND NOT CONFIG_KCL_LINUX)
  zephyr_library()
  zephyr_library_include_directories(${ZEPHYR_BASE}/kernel/include ${ZEPHYR_BASE}/arch/${ARCH}/include)
  zephyr_library_sources(kcl_default.c)
endif()
