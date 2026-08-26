# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared
  ${ZEPHYR_BASE}/kernel/include
  ${ARCH_DIR}/${ARCH}/include
)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)

if(NOT CONFIG_TC_PROVIDES_POSIX_SPAWN)
  zephyr_library_sources(
    posix_spawn.c
    posix_spawnp.c
    posix_spawn_file_actions_addclose.c
    posix_spawn_file_actions_adddup2.c
    posix_spawn_file_actions_addopen.c
    posix_spawn_file_actions_destroy.c
    posix_spawn_file_actions_init.c
    posix_spawnattr_destroy.c
    posix_spawnattr_getflags.c
    posix_spawnattr_getpgroup.c
    posix_spawnattr_getschedparam.c
    posix_spawnattr_getschedpolicy.c
    posix_spawnattr_getsigdefault.c
    posix_spawnattr_getsigmask.c
    posix_spawnattr_init.c
    posix_spawnattr_setflags.c
    posix_spawnattr_setpgroup.c
    posix_spawnattr_setschedparam.c
    posix_spawnattr_setschedpolicy.c
    posix_spawnattr_setsigdefault.c
    posix_spawnattr_setsigmask.c
  )
endif()
