.. _posix_option_group_device_specific:

POSIX_DEVICE_SPECIFIC
=====================

The ``POSIX_DEVICE_SPECIFIC`` Option Group is not yet supported in Zephyr. Zephyr has no controlling
terminal; the general terminal interfaces are unimplemented.

.. csv-table:: POSIX_DEVICE_SPECIFIC
   :header: API, Supported
   :widths: 50,10

    :c:func:`cfgetispeed`,no
    :c:func:`cfgetospeed`,no
    :c:func:`cfsetispeed`,no
    :c:func:`cfsetospeed`,no
    :c:func:`ctermid`,no
    :c:func:`isatty`,no
    :c:func:`tcdrain`,no
    :c:func:`tcflow`,no
    :c:func:`tcflush`,no
    :c:func:`tcgetattr`,no
    :c:func:`tcsendbreak`,no
    :c:func:`tcsetattr`,no
    :c:func:`ttyname`,no

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_DEVICE_SPECIFIC`` Option
Group.

.. doxygengroup:: posix_option_group_device_specific
   :project: posix
