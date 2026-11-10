.. _posix_options_posix2:

_POSIX2 Shell and Utilities Options
===================================

The ``_POSIX2_*`` symbolic constants describe features of the Shell and Utilities volume of
POSIX-1 (formerly POSIX.2), such as the ``c99`` development utility (``_POSIX2_C_DEV``), a
terminal capable of character-at-a-time input (``_POSIX2_CHAR_TERM``), the Fortran runtime
utilities (``_POSIX2_FORT_RUN``), the software development utilities (``_POSIX2_SW_DEV``),
and the User Portability Utilities (``_POSIX2_UPE``). The Shell and Utilities volume is
beyond the scope of Zephyr's POSIX support, so all of these are unsupported (-1), although
they are required by the :ref:`PSE54 <posix_aep_pse54>` profile.

.. csv-table:: _POSIX2 Options
   :header: Symbol, Support
   :widths: 50,10

    _POSIX2_C_DEV,-1
    _POSIX2_CHAR_TERM,-1
    _POSIX2_FORT_RUN,-1
    _POSIX2_SW_DEV,-1
    _POSIX2_UPE,-1
