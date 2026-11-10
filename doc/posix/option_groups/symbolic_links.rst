.. _posix_option_group_symbolic_links:

POSIX_SYMBOLIC_LINKS
====================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_SYMBOLIC_LINKS`.

Symbolic links require support from the underlying file system driver; ext2 implements them as
fast symbolic links (the target lives in the inode, at most 60 bytes). On file systems without
symbolic links, symlink() fails, readlink() reports EINVAL, and lstat() is equivalent to stat().

.. csv-table:: POSIX_SYMBOLIC_LINKS
   :header: API, Supported
   :widths: 50,10

    :c:func:`lchown`,yes
    :c:func:`lstat`,yes
    :c:func:`readlink`,yes
    :c:func:`symlink`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_SYMBOLIC_LINKS`` Option
Group.
