.. _posix_option_group_file_attributes:

POSIX_FILE_ATTRIBUTES
=====================

The ``POSIX_FILE_ATTRIBUTES`` Option Group is not yet supported in Zephyr. Zephyr file systems do not
implement owners, groups, or permission bits.

.. csv-table:: POSIX_FILE_ATTRIBUTES
   :header: API, Supported
   :widths: 50,10

    :c:func:`chmod`,
    :c:func:`chown`,
    :c:func:`fchmod`,
    :c:func:`fchown`,
    :c:func:`umask`,

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FILE_ATTRIBUTES`` Option
Group.
