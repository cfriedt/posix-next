.. _posix_option_group_user_groups_r:

POSIX_USER_GROUPS_R
===================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_USER_GROUPS_R`.

.. csv-table:: POSIX_USER_GROUPS_R
   :header: API, Supported
   :widths: 50,10

    :c:func:`getlogin_r`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_USER_GROUPS_R`` Option
Group.

.. doxygengroup:: posix_option_group_user_groups_r
   :project: posix
