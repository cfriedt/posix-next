.. _posix_option_group_multi_process:

POSIX_MULTI_PROCESS
===================

Enable this option group with :kconfig:option:`CONFIG_POSIX_MULTI_PROCESS`.

.. csv-table:: POSIX_MULTI_PROCESS
   :header: API, Supported
   :widths: 50,10

    :c:func:`_Exit`, yes
    :c:func:`_exit`, yes
    :c:func:`assert`, yes
    :c:func:`atexit`,yes
    :c:func:`clock`,
    :c:func:`execl`,yes
    :c:func:`execle`,yes
    :c:func:`execlp`,yes
    :c:func:`execv`,yes
    :c:func:`execve`,yes
    :c:func:`execvp`,yes
    :c:func:`exit`, yes
    :c:func:`fork`,yes
    :c:func:`getpgrp`,yes
    :c:func:`getpgid`,yes
    :c:func:`getpid`, yes
    :c:func:`getppid`,yes
    :c:func:`getsid`,yes
    :c:func:`setsid`,yes
    :c:func:`sleep`,yes
    :c:func:`times`,yes
    :c:func:`wait`,yes
    :c:func:`waitid`,yes
    :c:func:`waitpid`,yes

See :ref:`posix_multi_process_design` for implementation details.

.. doxygengroup:: posix_option_group_multi_process
   :project: posix

