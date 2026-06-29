.. _posix_option_group_system_database:

POSIX_SYSTEM_DATABASE
=====================

Enable this option group with :kconfig:option:`CONFIG_POSIX_SYSTEM_DATABASE`.

.. csv-table:: POSIX_SYSTEM_DATABASE
   :header: API, Supported
   :widths: 50,10

    :c:func:`getgrgid`,yes
    :c:func:`getgrnam`,yes
    :c:func:`getpwnam`,yes
    :c:func:`getpwuid`,yes

.. doxygengroup:: posix_option_group_system_database
   :project: posix

