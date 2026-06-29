.. _posix_option_thread_safe_functions:

_POSIX_THREAD_SAFE_FUNCTIONS
============================

In Zephyr, ``_POSIX_THREAD_SAFE_FUNCTIONS`` is defined when
:kconfig:option:`CONFIG_POSIX_C_LANG_SUPPORT_R`,
:kconfig:option:`CONFIG_POSIX_FILE_SYSTEM_R`, and
:kconfig:option:`CONFIG_POSIX_FILE_LOCKING` are enabled.

.. csv-table:: _POSIX_THREAD_SAFE_FUNCTIONS
   :header: API, Supported
   :widths: 50,10

    :c:func:`asctime_r`,yes
    :c:func:`ctime_r`,yes
    :c:func:`flockfile`,yes
    :c:func:`ftrylockfile`,yes
    :c:func:`funlockfile`,yes
    :c:func:`getc_unlocked`,yes
    :c:func:`getchar_unlocked`,yes
    :c:func:`getgrgid_r`,yes
    :c:func:`getgrnam_r`,yes
    :c:func:`getpwnam_r`,yes
    :c:func:`getpwuid_r`,yes
    :c:func:`gmtime_r`,yes
    :c:func:`localtime_r`,yes
    :c:func:`putc_unlocked`,yes
    :c:func:`putchar_unlocked`,yes
    :c:func:`rand_r`,yes
    :c:func:`readdir_r`,yes
    :c:func:`strerror_r`,yes
    :c:func:`strtok_r`,yes

.. doxygengroup:: posix_option_thread_safe_functions
   :project: posix

