.. _posix_option_group_single_process:

POSIX_SINGLE_PROCESS
====================

The POSIX_SINGLE_PROCESS option group contains services for single
process applications.

Enable this option group with :kconfig:option:`CONFIG_POSIX_SINGLE_PROCESS`.

.. csv-table:: POSIX_SINGLE_PROCESS
   :header: API, Supported
   :widths: 50,10

    :c:func:`confstr`,yes
    environ,yes
    errno,yes
    :c:func:`getenv`,yes
    :c:func:`setenv`,yes
    :c:func:`sysconf`,yes
    :c:func:`uname`,yes
    :c:func:`unsetenv`,yes

.. doxygengroup:: posix_option_group_single_process
   :project: posix

