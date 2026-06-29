.. _posix_option_group_xsi_system_logging:

XSI_SYSTEM_LOGGING
==================

Enable this option group with :kconfig:option:`CONFIG_XSI_SYSTEM_LOGGING`.

.. csv-table:: XSI_SYSTEM_LOGGING
   :header: API, Supported
   :widths: 50,10

    :c:func:`closelog`,yes
    :c:func:`openlog`,yes
    :c:func:`setlogmask`,yes
    :c:func:`syslog`,yes

.. doxygengroup:: posix_option_group_xsi_system_logging
   :project: posix

