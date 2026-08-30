.. _posix_option_group_shell_func:

POSIX_SHELL_FUNC
================

The ``POSIX_SHELL_FUNC`` Option Group is not yet supported in Zephyr. These interfaces require a
conformant shell (see :ref:`_POSIX_SHELL <posix_option_shell>`).

.. csv-table:: POSIX_SHELL_FUNC
   :header: API, Supported
   :widths: 50,10

    :c:func:`pclose`,no
    :c:func:`popen`,no
    :c:func:`system`,no
    :c:func:`wordexp`,no
    :c:func:`wordfree`,no

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_SHELL_FUNC`` Option
Group.
