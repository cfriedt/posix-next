.. _posix_option_thread_priority_scheduling:

_POSIX_THREAD_PRIORITY_SCHEDULING
=================================

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_PRIORITY_SCHEDULING`.

.. csv-table:: _POSIX_THREAD_PRIORITY_SCHEDULING
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_attr_getinheritsched`,yes
    :c:func:`pthread_attr_getschedpolicy`,yes
    :c:func:`pthread_attr_getscope`,yes
    :c:func:`pthread_attr_setinheritsched`,yes
    :c:func:`pthread_attr_setschedpolicy`,yes
    :c:func:`pthread_attr_setscope`,yes
    :c:func:`pthread_getschedparam`,yes
    :c:func:`pthread_setschedparam`,yes
    :c:func:`pthread_setschedprio`,yes

.. doxygengroup:: posix_option_thread_priority_scheduling
   :project: posix

