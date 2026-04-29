.. _posix_option_group_file_locking:

POSIX_FILE_LOCKING
==================

Enable this option group with :kconfig:option:`CONFIG_POSIX_FILE_LOCKING`.

.. csv-table:: POSIX_FILE_LOCKING
   :header: API, Supported
   :widths: 50,10

    :c:func:`flockfile`,yes
    :c:func:`ftrylockfile`,yes
    :c:func:`funlockfile`,yes
    :c:func:`getc_unlocked`,yes
    :c:func:`getchar_unlocked`,yes
    :c:func:`putc_unlocked`,yes
    :c:func:`putchar_unlocked`,yes

.. doxygengroup:: posix_option_group_file_locking
   :project: posix

