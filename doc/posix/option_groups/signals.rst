.. _posix_option_group_signals:

POSIX_SIGNALS
=============

Enable this option group with :kconfig:option:`CONFIG_POSIX_SIGNALS`.

.. csv-table:: POSIX_SIGNALS
   :header: API, Supported
   :widths: 50,10

    :c:func:`abort`,yes
    :c:func:`alarm`,yes
    :c:func:`kill`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pause`,yes
    :c:func:`raise`,yes
    :c:func:`sigaction`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigaddset`,yes
    :c:func:`sigdelset`,yes
    :c:func:`sigemptyset`,yes
    :c:func:`sigfillset`,yes
    :c:func:`sigismember`,yes
    :c:func:`signal`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigpending`,yes
    :c:func:`sigprocmask`,yes
    :c:func:`sigsuspend`,yes
    :c:func:`sigwait`,yes

See :ref:`Signal Implementation Details <posix_implementation_signals>` for more info.

.. doxygengroup:: posix_option_group_signals
   :project: posix
