.. _posix_option_thread_attr_stackaddr:

_POSIX_THREAD_ATTR_STACKADDR
============================

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKADDR`.

``pthread_attr_getstack()`` and ``pthread_attr_setstack()`` also require
:ref:`_POSIX_THREAD_ATTR_STACKSIZE <posix_option_thread_attr_stacksize>` /
:kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKSIZE`.

IEEE 1003.1-2017 removed ``pthread_attr_getstackaddr()`` and
``pthread_attr_setstackaddr()`` in favour of the combined stack APIs.

.. csv-table:: _POSIX_THREAD_ATTR_STACKADDR
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_attr_getstack`,yes
    :c:func:`pthread_attr_setstack`,yes

.. doxygengroup:: posix_option_thread_attr_stackaddr
   :project: posix
