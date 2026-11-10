.. _posix_option_group_symbolic_links_fd:

POSIX_SYMBOLIC_LINKS_FD
=======================

Enable this option group with :kconfig:option:`CONFIG_POSIX_SYMBOLIC_LINKS_FD`.

.. csv-table:: POSIX_SYMBOLIC_LINKS_FD
   :header: API, Supported
   :widths: 50,10

    :c:func:`readlinkat`, yes
    :c:func:`symlinkat`, yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_SYMBOLIC_LINKS_FD`` Option
Group.
