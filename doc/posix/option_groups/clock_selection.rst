.. _posix_option_group_clock_selection:

POSIX_CLOCK_SELECTION
=====================

Enable this option group with :kconfig:option:`CONFIG_POSIX_CLOCK_SELECTION`.

.. csv-table:: POSIX_CLOCK_SELECTION
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_condattr_getclock`,yes
    :c:func:`pthread_condattr_setclock`,yes
    :c:func:`clock_nanosleep`,yes

.. doxygengroup:: posix_option_group_clock_selection
   :project: posix

