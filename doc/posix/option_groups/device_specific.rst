.. _posix_option_group_device_specific:

POSIX_DEVICE_SPECIFIC
=====================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_DEVICE_SPECIFIC`.

The console is the one terminal. Attribute changes round-trip; the console driver is
unbuffered and synchronous, so the drain, flush, flow, and break operations complete
trivially. Attributes are not yet applied to the UART, and canonical input processing
awaits console input wiring.

.. csv-table:: POSIX_DEVICE_SPECIFIC
   :header: API, Supported
   :widths: 50,10

    :c:func:`cfgetispeed`,yes
    :c:func:`cfgetospeed`,yes
    :c:func:`cfsetispeed`,yes
    :c:func:`cfsetospeed`,yes
    :c:func:`ctermid`,yes
    :c:func:`isatty`,yes
    :c:func:`tcdrain`,yes
    :c:func:`tcflow`,yes
    :c:func:`tcflush`,yes
    :c:func:`tcgetattr`,yes
    :c:func:`tcsendbreak`,yes
    :c:func:`tcsetattr`,yes
    :c:func:`ttyname`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_DEVICE_SPECIFIC`` Option
Group.

.. doxygengroup:: posix_option_group_device_specific
   :project: posix
