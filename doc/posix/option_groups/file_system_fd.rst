.. _posix_option_group_file_system_fd:

POSIX_FILE_SYSTEM_FD
====================

Enable this option group with :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM_FD`.

A relative path resolves against the directory that the file descriptor argument refers to, or
against the working directory when it is ``AT_FDCWD``. The file system subsystem exposes no
hard-link operation, so linkat() fails with EPERM, as link() does.

.. csv-table:: POSIX_FILE_SYSTEM_FD
   :header: API, Supported
   :widths: 50,10

    :c:func:`faccessat`, yes
    :c:func:`fdopendir`, yes
    :c:func:`fstatat`, yes
    :c:func:`linkat`, yes
    :c:func:`mkdirat`, yes
    :c:func:`openat`, yes
    :c:func:`renameat`, yes
    :c:func:`unlinkat`, yes
    :c:func:`utimensat`, yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FILE_SYSTEM_FD`` Option
Group.
