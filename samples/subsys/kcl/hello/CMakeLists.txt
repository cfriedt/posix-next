# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(kcl_linux_hello)

# The guest is an ordinary Linux program, built for the host with the host's
# compiler: static and position independent, so the sample only runs on the
# qemu target of the host's own architecture.
set(GUEST "" CACHE FILEPATH "A prebuilt static-pie Linux executable to run instead")
if(GUEST)
  set(guest ${GUEST})
else()
  # the guest is a Linux program for the target: the host's own compiler when
  # the architectures agree, a Linux cross compiler otherwise
  if(CONFIG_X86_64)
    set(guest_cc_names x86_64-linux-gnu-gcc cc gcc)
  elseif(CONFIG_ARM64)
    set(guest_cc_names aarch64-linux-gnu-gcc)
  elseif(CONFIG_RISCV AND CONFIG_64BIT)
    set(guest_cc_names riscv64-linux-gnu-gcc)
  else()
    message(FATAL_ERROR "no Linux guest build for this architecture")
  endif()
  find_program(HOST_CC NAMES ${guest_cc_names})
  if(NOT HOST_CC)
    message(FATAL_ERROR "Linux compiler for the target not found (one of: ${guest_cc_names}); "
                        "install the distribution's cross toolchain, or pass -DGUEST=<prebuilt>")
  endif()
  set(guest ${CMAKE_CURRENT_BINARY_DIR}/hello)
  add_custom_command(
    OUTPUT ${guest}
    COMMAND ${HOST_CC} -static-pie -O2 -Wall -o ${guest} ${CMAKE_CURRENT_SOURCE_DIR}/guest/hello.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/guest/hello.c
    COMMENT "Building the Linux guest with ${HOST_CC}"
  )
endif()
# stripped and compressed: inflated into the root file system at boot
find_program(XZ xz REQUIRED)
set(guest_xz ${CMAKE_CURRENT_BINARY_DIR}/hello.xz)
add_custom_command(
  OUTPUT ${guest_xz}
  COMMAND ${CMAKE_STRIP} -o ${guest_xz}.elf ${guest}
  COMMAND sh -c "${XZ} --format=xz --check=crc32 --lzma2=preset=9e,dict=2MiB -c ${guest_xz}.elf > ${guest_xz}"
  DEPENDS ${guest}
  COMMENT "Stripping and compressing the Linux guest"
  VERBATIM
)
add_custom_target(guest DEPENDS ${guest} ${guest_xz})
add_dependencies(app guest)

target_sources(app PRIVATE src/main.c)
target_sources_ifdef(CONFIG_TRACING_USER app PRIVATE src/trace.c)
generate_inc_file_for_target(app ${guest_xz} ${ZEPHYR_BINARY_DIR}/include/generated/hello.inc)
