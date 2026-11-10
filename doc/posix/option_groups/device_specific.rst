.. _posix_option_group_device_specific:

POSIX_DEVICE_SPECIFIC
=====================

The ``POSIX_DEVICE_SPECIFIC`` Option Group is not yet supported in Zephyr. Zephyr has no controlling
terminal; the general terminal interfaces are unimplemented.

.. csv-table:: POSIX_DEVICE_SPECIFIC
   :header: API, Supported
   :widths: 50,10

    :c:func:`cfgetispeed`,
    :c:func:`cfgetospeed`,
    :c:func:`cfsetispeed`,
    :c:func:`cfsetospeed`,
    :c:func:`ctermid`,
    :c:func:`isatty`,
    :c:func:`tcdrain`,
    :c:func:`tcflow`,
    :c:func:`tcflush`,
    :c:func:`tcgetattr`,
    :c:func:`tcsendbreak`,
    :c:func:`tcsetattr`,
    :c:func:`ttyname`,

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_DEVICE_SPECIFIC`` Option
Group.

.. doxygengroup:: posix_option_group_device_specific
   :project: posix
