.. _posix_option_group_user_groups:

POSIX_USER_GROUPS
=================

The ``POSIX_USER_GROUPS`` Option Group is not yet supported in Zephyr. Zephyr is a single-user
system without user or group identities.

.. csv-table:: POSIX_USER_GROUPS
   :header: API, Supported
   :widths: 50,10

    :c:func:`getegid`,
    :c:func:`geteuid`,
    :c:func:`getgid`,
    :c:func:`getgroups`,
    :c:func:`getlogin`,
    :c:func:`getuid`,
    :c:func:`setegid`,
    :c:func:`seteuid`,
    :c:func:`setgid`,
    :c:func:`setuid`,

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_USER_GROUPS`` Option
Group.

.. doxygengroup:: posix_option_group_user_groups
   :project: posix
