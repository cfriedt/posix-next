.. _posix_options_posix2:

_POSIX2 Shell and Utilities Options
===================================

The ``_POSIX2_*`` symbolic constants describe features of the Shell and Utilities volume of
POSIX-1 (formerly POSIX.2): the mandatory utilities themselves (``_POSIX2_VERSION``), the ``c99``
development utilities (``_POSIX2_C_DEV``), a terminal capable of character-at-a-time input
(``_POSIX2_CHAR_TERM``), the Fortran utilities (``_POSIX2_FORT_DEV``, ``_POSIX2_FORT_RUN``),
locale creation (``_POSIX2_LOCALEDEF``), the software development utilities
(``_POSIX2_SW_DEV``) and the User Portability Utilities (``_POSIX2_UPE``). The
:ref:`PSE54 <posix_aep_pse54>` profile requires them.

The utilities are not part of this module. A module that provides a complete, conforming
utility set for an option, such as a multi-call binary run through the
:ref:`POSIX_SHELL_FUNC <posix_option_group_shell_func>` interfaces, selects the matching
non-user-selectable Kconfig symbol below, and ``<unistd.h>`` and :c:func:`sysconf` then report
the option. The options are all-or-nothing: a provider that lacks any utility of a set leaves
its symbol unselected and the option is reported as unsupported (-1). No provider claims one
today; the toybox module, for instance, still lacks ``ed``, ``gencat``, ``join``, ``lp``,
``m4``, ``mailx``, ``pathchk``, ``pax``, ``pr`` and the locale utilities of the mandatory set,
and its ``sh`` and ``awk`` are pending.

.. csv-table:: _POSIX2 Options
   :header: Symbol, Support, Remarks
   :widths: 50,10,50

    _POSIX2_VERSION,-1,:kconfig:option:`CONFIG_POSIX2_SHELL_UTILITIES`
    _POSIX2_C_BIND,200809L,
    _POSIX2_C_DEV,-1,:kconfig:option:`CONFIG_POSIX2_C_DEV`
    _POSIX2_CHAR_TERM,-1,:kconfig:option:`CONFIG_POSIX2_CHAR_TERM`
    _POSIX2_FORT_DEV,-1,:kconfig:option:`CONFIG_POSIX2_FORT_DEV`
    _POSIX2_FORT_RUN,-1,:kconfig:option:`CONFIG_POSIX2_FORT_RUN`
    _POSIX2_LOCALEDEF,-1,:kconfig:option:`CONFIG_POSIX2_LOCALEDEF`
    _POSIX2_SW_DEV,-1,:kconfig:option:`CONFIG_POSIX2_SW_DEV`
    _POSIX2_UPE,-1,:kconfig:option:`CONFIG_POSIX2_UPE`
