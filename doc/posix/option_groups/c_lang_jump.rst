.. _posix_option_group_c_lang_jump:

POSIX_C_LANG_JUMP
=================

The ``POSIX_C_LANG_JUMP`` Option Group is included in the ISO C standard.

.. note::
   When using Newlib, Picolibc, or other C libraries conforming to the ISO C Standard, the
   ``POSIX_C_LANG_JUMP`` Option Group is considered supported.

.. csv-table:: POSIX_C_LANG_JUMP
   :header: API, Supported
   :widths: 50,10

    :c:func:`setjmp`, yes
    :c:func:`longjmp`, yes

