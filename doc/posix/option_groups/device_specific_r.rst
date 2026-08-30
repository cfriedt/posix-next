.. _posix_option_group_device_specific_r:

POSIX_DEVICE_SPECIFIC_R
=======================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_DEVICE_SPECIFIC_R`.

.. csv-table:: POSIX_DEVICE_SPECIFIC_R
   :header: API, Supported
   :widths: 50,10

    :c:func:`ttyname_r`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_DEVICE_SPECIFIC_R`` Option
Group.

.. doxygengroup:: posix_option_group_device_specific_r
   :project: posix
