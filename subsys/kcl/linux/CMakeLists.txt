# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0

# The architecture's Linux system calls: declarations, ID reservations,
# verifiers and default implementations, generated from syscalls.json
set(gen_dir ${PROJECT_BINARY_DIR}/kcl/linux)
set(generator ${CMAKE_CURRENT_SOURCE_DIR}/gen_linux_syscalls.py)
set(syscalls_json ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.json)

execute_process(
  COMMAND ${PYTHON_EXECUTABLE} ${generator}
    emit --arch ${CONFIG_KCL_LINUX_ARCH} --json ${syscalls_json} --out ${gen_dir}
  RESULT_VARIABLE ret
)
if(NOT ret EQUAL 0)
  message(FATAL_ERROR "generating the Linux system calls failed: ${ret}")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${generator} ${syscalls_json})

zephyr_library()
zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ZEPHYR_BASE}/kernel/include
  ${ZEPHYR_BASE}/arch/${ARCH}/include
)

zephyr_include_directories(${gen_dir}/include)
zephyr_syscall_header(${gen_dir}/include/zephyr/kcl/linux/syscalls.h)
zephyr_syscall_reserve(${gen_dir}/syscalls.tbl)

zephyr_library_sources(
  ${gen_dir}/syscall_vrfy.c
  ${gen_dir}/syscall_defaults.c
  errno.c
  fs.c
  futex.c
  mm.c
  misc.c
  process.c
)
zephyr_library_sources_ifdef(CONFIG_X86_64 arch_x86_64.c)
zephyr_library_sources_ifdef(CONFIG_NET_SOCKETS net.c)

zephyr_syscall_header_ifdef(CONFIG_KCL_LINUX_EXEC ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/zephyr/kcl/linux/exec.h)
zephyr_library_sources_ifdef(CONFIG_KCL_LINUX_EXEC exec.c start.c)
