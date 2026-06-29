.. _posix_option_cputime:

_POSIX_CPUTIME
==============

Enable this option with :kconfig:option:`CONFIG_POSIX_CPUTIME`, or enable the
:ref:`XSI_ADVANCED_REALTIME <posix_option_group_xsi_advanced_realtime>` option group with
:kconfig:option:`CONFIG_XSI_ADVANCED_REALTIME`.

.. csv-table:: _POSIX_CPUTIME
   :header: API, Supported
   :widths: 50,10

    CLOCK_PROCESS_CPUTIME_ID,yes
    :c:func:`clock_getcpuclockid`,yes

.. doxygengroup:: posix_option_cputime
   :project: posix

