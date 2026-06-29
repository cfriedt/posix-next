.. _posix_option_group_xsi_threads_ext:

XSI_THREADS_EXT
===============

The ``XSI_THREADS_EXT`` option group provides thread concurrency hints.

Enable this option group with :kconfig:option:`CONFIG_XSI_THREADS_EXT`.

Combined stack address and size control is provided by
:ref:`pthread_attr_getstack() <posix_option_thread_attr_stackaddr>` and
:ref:`pthread_attr_setstack() <posix_option_thread_attr_stackaddr>` when both
:ref:`_POSIX_THREAD_ATTR_STACKADDR <posix_option_thread_attr_stackaddr>` and
:ref:`_POSIX_THREAD_ATTR_STACKSIZE <posix_option_thread_attr_stacksize>` are
enabled.

.. csv-table:: XSI_THREADS_EXT
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_getconcurrency`,yes
    :c:func:`pthread_setconcurrency`,yes

.. doxygengroup:: posix_option_group_xsi_threads_ext
   :project: posix
