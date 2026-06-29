.. _posix_option_group_signals:

POSIX_SIGNALS
=============

Enable this option group with :kconfig:option:`CONFIG_POSIX_SIGNALS`.

.. note::
   As processes are not yet supported in Zephyr, the ISO C functions ``abort()``, ``signal()``,
   and ``raise()``, as well as the other POSIX functions listed below, may exhibit undefined
   behaviour. The POSIX functions ``kill()``, ``pause()``, ``sigaction()``, ``sigpending()``,
   ``sigsuspend()``, and ``sigwait()`` are implemented to ensure that conformant applications can
   link, but they are expected to fail, setting errno to ``ENOSYS``
   :ref:`†<posix_undefined_behaviour>`.

.. csv-table:: POSIX_SIGNALS
   :header: API, Supported
   :widths: 50,10

    :c:func:`abort`, :ref:`†<posix_undefined_behaviour>`
    :c:func:`alarm`, :ref:`†<posix_undefined_behaviour>`
    :c:func:`kill`, :ref:`†<posix_undefined_behaviour>`
    :c:func:`pause`, :ref:`†<posix_undefined_behaviour>`
    :c:func:`raise`, :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigaction`, :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigaddset`,yes
    :c:func:`sigdelset`,yes
    :c:func:`sigemptyset`,yes
    :c:func:`sigfillset`,yes
    :c:func:`sigismember`,yes
    :c:func:`signal`, :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigpending`, :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigprocmask`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigsuspend`, :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigwait`, :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_group_signals
   :project: posix

