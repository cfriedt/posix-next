# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared
  ${CMAKE_CURRENT_SOURCE_DIR}/../timers
  ${ZEPHYR_BASE}/kernel/include
  ${ARCH_DIR}/${ARCH}/include
)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
zephyr_library_compile_options_ifdef(CONFIG_XSI -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

if(NOT CONFIG_TC_PROVIDES_POSIX_THREADS)
  zephyr_library_sources(cond.c grp.c key.c mutex.c pthread.c pwd.c)
  if(CONFIG_POSIX_THREAD_ATTR_STACKADDR AND CONFIG_POSIX_THREAD_ATTR_STACKSIZE)
    zephyr_library_sources(pthread_attr_stackaddr.c)
  endif()
  zephyr_library_sources_ifdef(CONFIG_POSIX_THREAD_ATTR_STACKSIZE pthread_attr_stacksize.c)
  zephyr_library_sources_ifdef(CONFIG_POSIX_THREAD_PRIO_INHERIT pthread_mutex_prio.c)
  zephyr_library_sources_ifdef(CONFIG_POSIX_THREAD_PRIO_PROTECT pthread_mutex_prio.c)
endif()
