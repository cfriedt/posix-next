.. _posix_option_group_shell_func:

POSIX_SHELL_FUNC
================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_SHELL_FUNC`.

The shell is toybox's, run as a process from the image the application installs at ``/bin/sh``
(see :zephyr:code-sample:`posix-toybox`); the group requires the toybox module. Each call spawns
``sh -c`` with the caller's environment, or with ``PATH=/bin`` alone when the application keeps
no environment. :c:func:`popen` connects a pipe to the command's standard input or output, and
:c:func:`wordexp` has the shell expand the words into its positional parameters and hand them
back over a pipe, so every expansion the shell performs is available; command substitution is
refused under ``WRDE_NOCMD`` before the shell runs. :c:func:`system` does not alter the caller's
signal dispositions around the wait, and ``WRDE_SHOWERR`` makes no difference: the shell's
error messages go to the console either way.

.. csv-table:: POSIX_SHELL_FUNC
   :header: API, Supported
   :widths: 50,10

    :c:func:`pclose`,yes
    :c:func:`popen`,yes
    :c:func:`system`,yes
    :c:func:`wordexp`,yes
    :c:func:`wordfree`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_SHELL_FUNC`` Option
Group.

.. doxygengroup:: posix_option_group_shell_func
   :project: posix
