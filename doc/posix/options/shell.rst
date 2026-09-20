.. _posix_option_shell:

_POSIX_SHELL
============

The ``_POSIX_SHELL`` Option does not add any interfaces; it indicates that the
:c:func:`system` function can execute shell commands with a conformant shell command
language interpreter. With :ref:`POSIX_SHELL_FUNC <posix_option_group_shell_func>` the
interpreter is toybox's shell, installed by the application at ``/bin/sh``.
