.. _posix_option_group_xsi_advanced_realtime:

XSI_ADVANCED_REALTIME
=====================

The ``XSI_ADVANCED_REALTIME`` option group indicates that the
:ref:`_POSIX_CLOCK_SELECTION<posix_option_group_clock_selection>`,
:ref:`_POSIX_CPUTIME<posix_option_cputime>`, and
:ref:`_POSIX_MONOTONIC_CLOCK<posix_option_monotonic_clock>` options are enabled.

Enable this option group with :kconfig:option:`CONFIG_XSI_ADVANCED_REALTIME`.

An implementation that claims conformance to this option group shall also support the
:ref:`POSIX_TIMERS <posix_option_group_timers>` option group.

