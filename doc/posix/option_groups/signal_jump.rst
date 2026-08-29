.. _posix_option_group_signal_jump:

POSIX_SIGNAL_JUMP
=================

Enable this option group with :kconfig:option:`CONFIG_POSIX_SIGNAL_JUMP`.

The implementation builds on the C library's ``setjmp()`` and ``longjmp()``
(which the minimal C library compiles from the picolibc module's machine
sources). Native builds support this option group only with the host C
library, whose ``setjmp()`` symbols any other configured C library would be
mismatched against.

.. csv-table:: POSIX_SIGNAL_JUMP
   :header: API, Supported
   :widths: 50,10

    :c:func:`siglongjmp`,yes
    :c:func:`sigsetjmp`,yes

