# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(kcl_linux_toybox)

# toybox as a Linux program: built for the host from a pristine export of the
# toybox module (its checkout carries the Zephyr platform patches), with the
# host's compiler and C library, static and position independent, so the
# sample only runs on the qemu target of the host's own architecture.
set(GUEST "" CACHE FILEPATH "A prebuilt static-pie toybox to run instead")
set(GUEST_APPLETS "" CACHE FILEPATH "The applets of a prebuilt toybox, one per line (default: ask it)")
if(GUEST)
  set(guest ${GUEST})
else()
  # toybox is a Linux program for the target: the host's own compiler when the
  # architectures agree, a Linux cross compiler with a static-PIE C library
  # otherwise
  if(CONFIG_X86_64)
    set(guest_cc_names x86_64-linux-gnu-gcc cc gcc)
  elseif(CONFIG_ARM64)
    set(guest_cc_names aarch64-linux-gnu-gcc)
  elseif(CONFIG_RISCV AND CONFIG_64BIT)
    set(guest_cc_names riscv64-linux-gnu-gcc)
  else()
    message(FATAL_ERROR "no Linux toybox build for this architecture")
  endif()
  find_program(HOST_CC NAMES ${guest_cc_names})
  if(NOT HOST_CC)
    message(FATAL_ERROR "Linux compiler for the target not found (one of: ${guest_cc_names}); "
                        "install the distribution's cross toolchain, or pass -DGUEST=<prebuilt>")
  endif()
  find_program(BUILD_CC NAMES cc gcc REQUIRED)
  find_program(GIT git REQUIRED)
  execute_process(COMMAND uname -srm OUTPUT_VARIABLE host_uname OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "toybox: compiled for the target with ${HOST_CC} on the build machine, ${host_uname}")
  if(NOT ZEPHYR_TOYBOX_MODULE_DIR)
    message(FATAL_ERROR "the toybox module is not in the workspace")
  endif()
  set(tb_src ${CMAKE_CURRENT_BINARY_DIR}/toybox-src)
  set(guest ${tb_src}/toybox)
  add_custom_command(
    OUTPUT ${guest}
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${tb_src}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${tb_src}
    COMMAND sh -c "${GIT} -C ${ZEPHYR_TOYBOX_MODULE_DIR} archive --format=tar HEAD | tar -x -C ${tb_src}"
    # the pending vi applet loses its buffer after :w (upstream toybox 0.8.14)
    COMMAND patch -s -p1 -d ${tb_src} -i ${CMAKE_CURRENT_SOURCE_DIR}/guest/vi-write-reload.patch
    COMMAND ${CMAKE_COMMAND} -E env --unset=CROSS_COMPILE CC=${HOST_CC} HOSTCC=${BUILD_CC}
            make -C ${tb_src} -s defconfig
    # the shell and the editor in, the applets that need libcrypt and /etc/shadow out
    COMMAND sed -i -E -e "s/^# CONFIG_SH is not set/CONFIG_SH=y/" -e "s/^# CONFIG_VI is not set/CONFIG_VI=y/"
                      -e "s/^CONFIG_(MKPASSWD|LOGIN|SU|PASSWD|CHSH|SULOGIN)=y/# CONFIG_\\1 is not set/" ${tb_src}/.config
    COMMAND ${CMAKE_COMMAND} -E env --unset=CROSS_COMPILE CC=${HOST_CC} HOSTCC=${BUILD_CC}
            CFLAGS=-fPIE LDFLAGS=-static-pie make -C ${tb_src} -s
    # the applets it was built with, from the build's own host-side lister
    COMMAND sh -c "${CMAKE_COMMAND} -E env --unset=CROSS_COMPILE CC=${HOST_CC} HOSTCC=${BUILD_CC} make -C ${tb_src} -s list | tr ' ' '\\n' | grep . > ${tb_src}/applets.txt"
    DEPENDS ${ZEPHYR_TOYBOX_MODULE_DIR}/toys.h ${ZEPHYR_TOYBOX_MODULE_DIR}/main.c
            ${CMAKE_CURRENT_SOURCE_DIR}/guest/vi-write-reload.patch
    COMMENT "Building toybox for the target with ${HOST_CC}"
    VERBATIM
  )
  set(GUEST_APPLETS ${tb_src}/applets.txt)
endif()
# the applets: a list file, else what toybox prints when run without arguments
set(commands_inc ${ZEPHYR_BINARY_DIR}/include/generated/toybox_commands.inc)
if(GUEST_APPLETS)
  set(list_applets "cat ${GUEST_APPLETS}")
else()
  set(list_applets ${guest})
endif()
add_custom_command(
  OUTPUT ${commands_inc}
  COMMAND sh -c "${list_applets} | tr ' ' '\\n' | grep . | sed 's/.*/\"&\",/' > ${commands_inc}"
  DEPENDS ${guest}
  COMMENT "Listing the toybox applets"
  VERBATIM
)
# stripped and compressed: the image carries the .xz stream and inflates it
# into the root file system at boot (a dictionary the size of the data loses
# nothing against a larger one and is what the decoder allocates)
find_program(XZ xz REQUIRED)
set(guest_xz ${CMAKE_CURRENT_BINARY_DIR}/toybox.xz)
add_custom_command(
  OUTPUT ${guest_xz}
  COMMAND ${CMAKE_STRIP} -o ${guest_xz}.elf ${guest}
  COMMAND sh -c "${XZ} --format=xz --check=crc32 --lzma2=preset=9e,dict=2MiB -c ${guest_xz}.elf > ${guest_xz}"
  DEPENDS ${guest}
  COMMENT "Stripping and compressing toybox"
  VERBATIM
)
add_custom_target(guest DEPENDS ${guest} ${guest_xz} ${commands_inc})
add_dependencies(app guest)

target_sources(app PRIVATE src/main.c src/fatal.c)
target_sources_ifdef(CONFIG_TRACING_USER app PRIVATE ../hello/src/trace.c)
generate_inc_file_for_target(app ${guest_xz} ${ZEPHYR_BINARY_DIR}/include/generated/toybox.inc)
generate_inc_file_for_target(app ${CMAKE_CURRENT_SOURCE_DIR}/guest/demo.sh
                             ${ZEPHYR_BINARY_DIR}/include/generated/demo_sh.inc)
