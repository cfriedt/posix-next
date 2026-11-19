.. _posix_option_group_file_system_ext:

POSIX_FILE_SYSTEM_EXT
=====================

Enable this option group with :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM_EXT`.

:c:func:`getdelim` and :c:func:`getline` allocate through the C library's :c:func:`malloc` and
accept a null buffer pointer regardless of the value behind ``n``, as POSIX requires.
:c:func:`scandir` entries and the entry array are individually allocated and are the caller's
to free. Zephyr's file systems report no ``.`` or ``..`` entries, so scandir() does not return
them.

.. csv-table:: POSIX_FILE_SYSTEM_EXT
   :header: API, Supported
   :widths: 50,10

    :c:func:`alphasort`, yes
    :c:func:`dirfd`, yes
    :c:func:`getdelim`, yes
    :c:func:`getline`, yes
    :c:func:`mkdtemp`, yes
    :c:func:`scandir`, yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FILE_SYSTEM_EXT`` Option
Group.
