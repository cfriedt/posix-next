.. _posix_option_group_barriers:

POSIX_BARRIERS
==============

Enable this option group with :kconfig:option:`CONFIG_POSIX_BARRIERS`.

.. csv-table:: POSIX_BARRIERS
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_barrier_destroy`,yes
    :c:func:`pthread_barrier_init`,yes
    :c:func:`pthread_barrier_wait`,yes
    :c:func:`pthread_barrierattr_destroy`,yes
    :c:func:`pthread_barrierattr_getpshared`,yes
    :c:func:`pthread_barrierattr_init`,yes
    :c:func:`pthread_barrierattr_setpshared`,yes

.. doxygengroup:: posix_option_group_barriers
   :project: posix

