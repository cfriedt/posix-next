.. _posix_option_group_file_attributes_fd:

POSIX_FILE_ATTRIBUTES_FD
========================

Enable this option group with :kconfig:option:`CONFIG_POSIX_FILE_ATTRIBUTES_FD`.

Like chmod() and chown(), these validate their arguments against a file system that keeps
neither permissions nor ownership.

.. csv-table:: POSIX_FILE_ATTRIBUTES_FD
   :header: API, Supported
   :widths: 50,10

    :c:func:`fchmodat`, yes
    :c:func:`fchownat`, yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FILE_ATTRIBUTES_FD`` Option
Group.
