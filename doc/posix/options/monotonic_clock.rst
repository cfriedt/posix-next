.. _posix_option_monotonic_clock:

_POSIX_MONOTONIC_CLOCK
======================

.. note::
   In Issue 7 (200809L), ``_POSIX_MONOTONIC_CLOCK`` was an optional option. Issue 8 (202405L)
   promoted it to a mandatory POSIX system-interface option.

Enable this option with :kconfig:option:`CONFIG_POSIX_MONOTONIC_CLOCK`, or enable the
:ref:`XSI_ADVANCED_REALTIME <posix_option_group_xsi_advanced_realtime>` option group with
:kconfig:option:`CONFIG_XSI_ADVANCED_REALTIME`.

.. csv-table:: _POSIX_MONOTONIC_CLOCK
   :header: API, Supported
   :widths: 50,10

    CLOCK_MONOTONIC,yes

.. doxygengroup:: posix_option_monotonic_clock
   :project: posix

