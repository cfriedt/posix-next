.. _posix_option_thread_cputime:

_POSIX_THREAD_CPUTIME
=====================

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_CPUTIME`.

.. csv-table:: _POSIX_THREAD_CPUTIME
   :header: API, Supported
   :widths: 50,10

    CLOCK_THREAD_CPUTIME_ID,yes
    :c:func:`pthread_getcpuclockid`,yes :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_thread_cputime
   :project: posix

