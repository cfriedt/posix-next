.. _posix_option_group_spin_locks:

POSIX_SPIN_LOCKS
================

Enable this option group with :kconfig:option:`CONFIG_POSIX_SPIN_LOCKS`.

.. csv-table:: POSIX_SPIN_LOCKS
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_spin_destroy`,yes
    :c:func:`pthread_spin_init`,yes
    :c:func:`pthread_spin_lock`,yes
    :c:func:`pthread_spin_trylock`,yes
    :c:func:`pthread_spin_unlock`,yes

.. doxygengroup:: posix_option_group_spin_locks
   :project: posix

