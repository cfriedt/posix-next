.. _posix_option_group_regexp:

POSIX_REGEXP
============

Enable this option group with :kconfig:option:`CONFIG_POSIX_REGEXP`.

Zephyr provides this option group with Henry Spencer's regex engine — the same engine Newlib
and Picolibc ship — so matching behaves identically regardless of the configured C library.

.. csv-table:: POSIX_REGEXP
   :header: API, Supported
   :widths: 50,10

    :c:func:`regcomp`,yes
    :c:func:`regerror`,yes
    :c:func:`regexec`,yes
    :c:func:`regfree`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_REGEXP`` Option
Group.
