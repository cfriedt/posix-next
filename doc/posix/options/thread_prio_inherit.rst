.. _posix_option_thread_prio_inherit:

_POSIX_THREAD_PRIO_INHERIT
==========================

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_PRIO_INHERIT`.

.. csv-table:: _POSIX_THREAD_PRIO_INHERIT
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_mutexattr_getprotocol`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_setprotocol`,yes :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_thread_prio_inherit
   :project: posix

