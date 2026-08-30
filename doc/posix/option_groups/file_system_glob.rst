.. _posix_option_group_file_system_glob:

POSIX_FILE_SYSTEM_GLOB
======================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM_GLOB`.

Pattern matching within a path segment uses fnmatch() from
:ref:`POSIX_C_LIB_EXT <posix_option_group_c_lib_ext>`. The structure layout and flag values
follow the BSD <glob.h> that Newlib and Picolibc ship, including the GLOB_NOESCAPE extension.

.. csv-table:: POSIX_FILE_SYSTEM_GLOB
   :header: API, Supported
   :widths: 50,10

    :c:func:`glob`,yes
    :c:func:`globfree`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FILE_SYSTEM_GLOB`` Option
Group.
