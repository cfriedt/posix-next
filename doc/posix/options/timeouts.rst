.. _posix_option_timeouts:

_POSIX_TIMEOUTS
===============

Enable this option with :kconfig:option:`CONFIG_POSIX_TIMERS`.

.. csv-table:: _POSIX_TIMEOUTS
   :header: API, Supported
   :widths: 50,10

    :c:func:`mq_timedreceive`, yes
    :c:func:`mq_timedsend`, yes
    :c:func:`pthread_mutex_timedlock`, yes
    :c:func:`pthread_rwlock_timedrdlock`, yes
    :c:func:`pthread_rwlock_timedwrlock`, yes
    :c:func:`sem_timedwait`, yes

.. doxygengroup:: posix_option_timeouts
   :project: posix

