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
  zephyr_library_sources(
    pthread_atfork.c
    pthread_attr_destroy.c
    pthread_attr_getdetachstate.c
    pthread_attr_getschedparam.c
    pthread_attr_init.c
    pthread_attr_setdetachstate.c
    pthread_attr_setschedparam.c
    pthread_cancel.c
    pthread_cond_broadcast.c
    pthread_cond_clockwait.c
    pthread_cond_destroy.c
    pthread_cond_init.c
    pthread_cond_signal.c
    pthread_cond_timedwait.c
    pthread_cond_wait.c
    pthread_condattr_destroy.c
    pthread_condattr_init.c
    pthread_create.c
    pthread_detach.c
    pthread_equal.c
    pthread_exit.c
    pthread_getspecific.c
    pthread_join.c
    pthread_key_create.c
    pthread_key_delete.c
    pthread_kill.c
    pthread_mutex_destroy.c
    pthread_mutex_init.c
    pthread_mutex_lock.c
    pthread_mutex_timedlock.c
    pthread_mutex_trylock.c
    pthread_mutex_unlock.c
    pthread_mutexattr_destroy.c
    pthread_mutexattr_init.c
    pthread_once.c
    pthread_self.c
    pthread_setcancelstate.c
    pthread_setcanceltype.c
    pthread_setspecific.c
    pthread_sigmask.c
    pthread_testcancel.c
    sched_yield.c
  )
  if(CONFIG_POSIX_THREAD_ATTR_STACKADDR AND CONFIG_POSIX_THREAD_ATTR_STACKSIZE)
    zephyr_library_sources(
      pthread_attr_getstack.c
      pthread_attr_setstack.c
    )
  endif()
  zephyr_library_sources_ifdef(CONFIG_POSIX_THREAD_ATTR_STACKSIZE
    pthread_attr_getstacksize.c
    pthread_attr_setstacksize.c
  )
  if(CONFIG_POSIX_THREAD_PRIO_INHERIT OR CONFIG_POSIX_THREAD_PRIO_PROTECT)
    zephyr_library_sources(
      pthread_mutexattr_getprotocol.c
      pthread_mutexattr_setprotocol.c
    )
  endif()
  zephyr_library_sources_ifdef(CONFIG_POSIX_THREAD_PRIO_PROTECT
    pthread_mutex_getprioceiling.c
    pthread_mutex_setprioceiling.c
    pthread_mutexattr_getprioceiling.c
    pthread_mutexattr_setprioceiling.c
  )
endif()
