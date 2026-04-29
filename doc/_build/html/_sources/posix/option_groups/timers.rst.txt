.. _posix_option_group_timers:

POSIX_TIMERS
============

Enable this option group with :kconfig:option:`CONFIG_POSIX_TIMERS`.

.. csv-table:: POSIX_TIMERS
   :header: API, Supported
   :widths: 50,10

    :c:func:`clock_getres`,yes
    :c:func:`clock_gettime`,yes
    :c:func:`clock_settime`,yes
    :c:func:`nanosleep`,yes
    :c:func:`timer_create`,yes
    :c:func:`timer_delete`,yes
    :c:func:`timer_gettime`,yes
    :c:func:`timer_getoverrun`,yes
    :c:func:`timer_settime`,yes

.. doxygengroup:: posix_option_group_timers
   :project: posix

