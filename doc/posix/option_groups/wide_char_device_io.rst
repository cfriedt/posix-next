.. _posix_option_group_wide_char_device_io:

POSIX_WIDE_CHAR_DEVICE_IO
=========================

The ``POSIX_WIDE_CHAR_DEVICE_IO`` Option Group is included in the ISO C standard.

Newlib and Picolibc provide it over the streams of
:ref:`POSIX_DEVICE_IO <posix_option_group_device_io>`; the minimal C library has no
wide-character support. Picolibc's :c:func:`fwide` reports a stream that has not been oriented
as byte-oriented and lets a later call change an orientation already set.

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_WIDE_CHAR_DEVICE_IO`` Option
Group.
