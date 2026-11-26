.. _posix_option_group_file_attributes_fd:

POSIX_FILE_ATTRIBUTES_FD
========================

Enable this option group with :kconfig:option:`CONFIG_POSIX_FILE_ATTRIBUTES_FD`.

The chmod() and chown() of a path relative to an open directory; ``AT_SYMLINK_NOFOLLOW`` changes
the owner of a symbolic link itself and refuses to change its mode (``EOPNOTSUPP``), as a link
has none of its own. See :ref:`posix_file_attributes_design`.

.. csv-table:: POSIX_FILE_ATTRIBUTES_FD
   :header: API, Supported
   :widths: 50,10

    :c:func:`fchmodat`, yes
    :c:func:`fchownat`, yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FILE_ATTRIBUTES_FD`` Option
Group.
