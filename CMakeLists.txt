# SPDX-License-Identifier: Apache-2.0

add_subdirectory(lib/posix)
add_subdirectory_ifdef(CONFIG_XZ_DEC lib/xz)
add_subdirectory(subsys/kcl)

zephyr_include_directories(include)
