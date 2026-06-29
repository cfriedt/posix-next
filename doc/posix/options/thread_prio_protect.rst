.. _posix_option_thread_prio_protect:

_POSIX_THREAD_PRIO_PROTECT
==========================

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_PRIO_PROTECT`.

.. csv-table:: _POSIX_THREAD_PRIO_PROTECT
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_mutex_getprioceiling`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutex_setprioceiling`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_getprioceiling`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_getprotocol`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_setprioceiling`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_setprotocol`,yes :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_thread_prio_protect
   :project: posix

