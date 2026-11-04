.. _posix_option_group_xsi_signals:

XSI_SIGNALS
===========

Enable this option group with :kconfig:option:`CONFIG_XSI_SIGNALS`.

.. csv-table:: XSI_SIGNALS
   :header: API, Supported
   :widths: 50,10

    :c:func:`killpg`,yes
    :c:func:`sigaltstack`,yes
    :c:func:`sighold`,yes
    :c:func:`sigignore`,yes
    :c:func:`siginterrupt`,yes
    :c:func:`sigpause`,yes
    :c:func:`sigrelse`,yes
    :c:func:`sigset`,yes

An alternate signal stack applies to user threads only (see
:kconfig:option:`CONFIG_SIGNAL_ALTSTACK`): for any other thread :c:func:`sigaltstack` fails with
``ENOTSUP``. A handler whose frame cannot be written on the stack it is to run on, or a fault taken
while a fault's handler is being entered or runs, ends the process with ``SIGSEGV``, as on Linux.
:c:func:`siginterrupt` has no effect: the kernel restarts no interrupted call, which fails with
``EINTR`` whatever the flag.

.. doxygengroup:: posix_option_group_xsi_signals
   :project: posix
