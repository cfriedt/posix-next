.. _posix_option_reader_writer_locks:

.. _posix_option_group_rw_locks:

POSIX_RW_LOCKS
==============

Enable this option with :kconfig:option:`CONFIG_POSIX_RW_LOCKS`.

.. csv-table:: POSIX_RW_LOCKS
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_rwlock_destroy`,yes
    :c:func:`pthread_rwlock_init`,yes
    :c:func:`pthread_rwlock_rdlock`,yes
    :c:func:`pthread_rwlock_tryrdlock`,yes
    :c:func:`pthread_rwlock_trywrlock`,yes
    :c:func:`pthread_rwlock_unlock`,yes
    :c:func:`pthread_rwlock_wrlock`,yes
    :c:func:`pthread_rwlockattr_destroy`,yes
    :c:func:`pthread_rwlockattr_getpshared`,yes
    :c:func:`pthread_rwlockattr_init`,yes
    :c:func:`pthread_rwlockattr_setpshared`,yes

.. doxygengroup:: posix_option_group_rw_locks
   :project: posix

