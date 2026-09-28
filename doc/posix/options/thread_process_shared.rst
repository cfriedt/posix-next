.. _posix_option_thread_process_shared:

_POSIX_THREAD_PROCESS_SHARED
============================

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_PROCESS_SHARED`; the X/Open System
Interfaces (:kconfig:option:`CONFIG_XSI`) require it and select it.

.. csv-table:: _POSIX_THREAD_PROCESS_SHARED
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_condattr_getpshared`,yes
    :c:func:`pthread_condattr_setpshared`,yes
    :c:func:`pthread_mutexattr_getpshared`,yes
    :c:func:`pthread_mutexattr_setpshared`,yes

The ``pshared`` attribute of barriers, read-write locks and spin locks, and the ``pshared``
argument of :c:func:`sem_init`, are accepted by their own option groups when this option is
enabled. See :ref:`posix_process_shared_synchronization` for how a shared object works.

.. doxygengroup:: posix_option_thread_process_shared
   :project: posix
