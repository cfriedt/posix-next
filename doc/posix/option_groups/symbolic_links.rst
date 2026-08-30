.. _posix_option_group_symbolic_links:

POSIX_SYMBOLIC_LINKS
====================

The ``POSIX_SYMBOLIC_LINKS`` Option Group is not yet supported in Zephyr. Zephyr file systems do not
implement symbolic links.

.. csv-table:: POSIX_SYMBOLIC_LINKS
   :header: API, Supported
   :widths: 50,10

    :c:func:`lchown`,no
    :c:func:`lstat`,no
    :c:func:`readlink`,no
    :c:func:`symlink`,no

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_SYMBOLIC_LINKS`` Option
Group.
