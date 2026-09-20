.. _posix_option_group_fifo:

POSIX_FIFO
==========

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_FIFO`.

A FIFO is a special file the file system stores; ext2 provides it. Opening one connects the
caller to the pipe the node names: a reader waits for a writer and a writer for a reader,
unless ``O_NONBLOCK`` is given, in which case a reader proceeds and a writer fails with
``ENXIO``. Once the last writer closes, readers see end-of-file after draining what was
written, and a later writer starts the exchange again. The pipes come from the pool sized by
:kconfig:option:`CONFIG_ZVFS_PIPE_MAX`, the open descriptions from
:kconfig:option:`CONFIG_ZVFS_FIFO_OPEN_MAX`. Opening a FIFO for both reading and writing is
undefined by POSIX and refused here. A FIFO removed while open keeps its pipe until the last
close; a new FIFO created at the same path meanwhile shares that pipe.

.. csv-table:: POSIX_FIFO
   :header: API, Supported
   :widths: 50,10

    :c:func:`mkfifo`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FIFO`` Option
Group.
