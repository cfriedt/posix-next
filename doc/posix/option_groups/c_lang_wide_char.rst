.. _posix_option_group_c_lang_wide_char:

POSIX_C_LANG_WIDE_CHAR
======================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_C_LANG_WIDE_CHAR`.

The ``POSIX_C_LANG_WIDE_CHAR`` Option Group is included in the ISO C standard: Newlib and
Picolibc provide it, and Zephyr adds the :c:func:`wcstold` that the Zephyr SDK's prebuilt
Picolibc omits. The minimal C library has no wide-character support.

The toolchain libraries deviate from ISO C in two places: they are built for the C locale
only, so multibyte sequences are single bytes, and :c:func:`swprintf` reports the length a
truncated result would have needed rather than a negative value. On IA-32, Zephyr's soft-float
stubs make ``long double`` arithmetic, and so :c:func:`wcstold`, unavailable.

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_C_LANG_WIDE_CHAR`` Option
Group.
