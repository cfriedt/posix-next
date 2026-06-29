.. _posix_option_group_system_database_r:

POSIX_SYSTEM_DATABASE_R
=======================

Enable this option group with :kconfig:option:`CONFIG_POSIX_SYSTEM_DATABASE_R`.

.. csv-table:: POSIX_SYSTEM_DATABASE_R
   :header: API, Supported
   :widths: 50,10

    :c:func:`getgrgid_r`,yes
    :c:func:`getgrnam_r`,yes
    :c:func:`getpwnam_r`,yes
    :c:func:`getpwuid_r`,yes

.. doxygengroup:: posix_option_group_system_database_r
   :project: posix

