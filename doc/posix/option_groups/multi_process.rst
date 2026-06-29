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
    :c:func:`atexit`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`clock`,
    :c:func:`execl`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execle`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execlp`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execv`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execve`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execvp`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`exit`, yes
    :c:func:`fork`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`getpgrp`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`getpgid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`getpid`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`getppid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`getsid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`setsid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`sleep`,yes
    :c:func:`times`,yes
    :c:func:`wait`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`waitid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`waitpid`,:ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_group_multi_process
   :project: posix

