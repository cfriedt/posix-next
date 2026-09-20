.. _posix_option_group_fifo_fd:

POSIX_FIFO_FD
=============

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_FIFO_FD`.

The directory-relative form of :ref:`POSIX_FIFO <posix_option_group_fifo>`. :c:func:`mknodat`
creates a FIFO for ``S_IFIFO`` and an empty regular file for ``S_IFREG``; no device special
files exist to make, so any other type is refused with ``EINVAL``.

.. csv-table:: POSIX_FIFO_FD
   :header: API, Supported
   :widths: 50,10

    :c:func:`mkfifoat`,yes
    :c:func:`mknodat`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FIFO_FD`` Option
Group.
