.. _posix_option_group_threads_base:

POSIX_THREADS_BASE
==================

The basic assumption in this profile is that the system
consists of a single (implicit) process with multiple threads. Therefore, the
standard requires all basic thread services, except those related to
multiple processes.

Enable this option group with :kconfig:option:`CONFIG_POSIX_THREADS`.

.. note::
   :c:func:`sched_yield` was added to the ``POSIX_THREADS_BASE`` option group in
   Issue 8 (202405L). It was not part of this option group in Issue 7 (200809L).

.. csv-table:: POSIX_THREADS_BASE
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_atfork`,yes
    :c:func:`pthread_attr_destroy`,yes
    :c:func:`pthread_attr_getdetachstate`,yes
    :c:func:`pthread_attr_getschedparam`,yes
    :c:func:`pthread_attr_init`,yes
    :c:func:`pthread_attr_setdetachstate`,yes
    :c:func:`pthread_attr_setschedparam`,yes
    :c:func:`pthread_cancel`,yes
    :c:func:`pthread_cleanup_pop`,yes
    :c:func:`pthread_cleanup_push`,yes
    :c:func:`pthread_cond_broadcast`,yes
    :c:func:`pthread_cond_destroy`,yes
    :c:func:`pthread_cond_init`,yes
    :c:func:`pthread_cond_signal`,yes
    :c:func:`pthread_cond_timedwait`,yes
    :c:func:`pthread_cond_wait`,yes
    :c:func:`pthread_condattr_destroy`,yes
    :c:func:`pthread_condattr_init`,yes
    :c:func:`pthread_create`,yes
    :c:func:`pthread_detach`,yes
    :c:func:`pthread_equal`,yes
    :c:func:`pthread_exit`,yes
    :c:func:`pthread_getspecific`,yes
    :c:func:`pthread_join`,yes
    :c:func:`pthread_key_create`,yes
    :c:func:`pthread_key_delete`,yes
    :c:func:`pthread_kill`,yes
    :c:func:`pthread_mutex_destroy`,yes
    :c:func:`pthread_mutex_init`,yes
    :c:func:`pthread_mutex_lock`,yes
    :c:func:`pthread_mutex_timedlock`,yes
    :c:func:`pthread_mutex_trylock`,yes
    :c:func:`pthread_mutex_unlock`,yes
    :c:func:`pthread_mutexattr_destroy`,yes
    :c:func:`pthread_mutexattr_init`,yes
    :c:func:`pthread_once`,yes
    :c:func:`pthread_self`,yes
    :c:func:`pthread_setcancelstate`,yes
    :c:func:`pthread_setcanceltype`,yes
    :c:func:`pthread_setspecific`,yes
    :c:func:`pthread_sigmask`,yes
    :c:func:`pthread_testcancel`,yes
    :c:func:`sched_yield`,yes

.. doxygengroup:: posix_option_group_threads_base
   :project: posix

