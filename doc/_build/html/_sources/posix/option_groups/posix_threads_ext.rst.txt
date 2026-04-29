.. _posix_option_group_posix_threads_ext:

POSIX_THREADS_EXT
=================

Enable this option group with :kconfig:option:`CONFIG_POSIX_THREADS_EXT`.

.. csv-table:: POSIX_THREADS_EXT
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_attr_getguardsize`,yes
    :c:func:`pthread_attr_setguardsize`,yes
    :c:func:`pthread_mutexattr_gettype`,yes
    :c:func:`pthread_mutexattr_settype`,yes

.. doxygengroup:: posix_option_group_posix_threads_ext
   :project: posix

