.. _posix_option_thread_attr_stacksize:

_POSIX_THREAD_ATTR_STACKSIZE
============================

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKSIZE`.

.. csv-table:: _POSIX_THREAD_ATTR_STACKSIZE
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_attr_getstacksize`,yes
    :c:func:`pthread_attr_setstacksize`,yes

.. doxygengroup:: posix_option_thread_attr_stacksize
   :project: posix

