.. _posix_option_group_file_attributes:

POSIX_FILE_ATTRIBUTES
=====================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_FILE_ATTRIBUTES`.

Modes and owners are stored by the file system: tmpfs and ext2 record them, while a file system
that records none (FAT, LittleFS) fails these calls with ``ENOTSUP`` and reports a synthesised
mode. The file mode creation mask is per process. See :ref:`posix_file_attributes_design`.

.. csv-table:: POSIX_FILE_ATTRIBUTES
   :header: API, Supported
   :widths: 50,10

    :c:func:`chmod`,yes
    :c:func:`chown`,yes
    :c:func:`fchmod`,yes
    :c:func:`fchown`,yes
    :c:func:`umask`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FILE_ATTRIBUTES`` Option
Group.
