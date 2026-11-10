.. _posix_option_group_device_specific:

POSIX_DEVICE_SPECIFIC
=====================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_DEVICE_SPECIFIC`.

The console is the one terminal. Attribute changes round-trip; the console driver is
unbuffered and synchronous, so the drain, flush, flow, and break operations complete
trivially. The canonical, echo and CR-to-NL input modes reach the console; the other
attributes are kept for tcgetattr() but not applied to the UART. With
:kconfig:option:`CONFIG_POSIX_DEVICE_SPECIFIC_SHARED_IO_BAUD` (the default) the terminal has one
baud rate: cfsetispeed() and cfsetospeed() set it for both directions, as on Linux, and an input
speed of zero follows the output speed; without it the two speeds are independent.

Only the console is a terminal. Bringing other Zephyr devices, UARTs among them, under the
``<termios.h>`` API would need something like `posix_devctl()
<https://pubs.opengroup.org/onlinepubs/9799919799.2024edition/functions/posix_devctl.html>`_
in the driver model, optionally, and is outside the scope of this Option Group's implementation.

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
