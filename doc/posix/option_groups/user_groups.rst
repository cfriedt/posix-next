.. _posix_option_group_user_groups:

POSIX_USER_GROUPS
=================

The ``POSIX_USER_GROUPS`` Option Group is not yet supported in Zephyr. Zephyr is a single-user
system without user or group identities.

.. csv-table:: POSIX_USER_GROUPS
   :header: API, Supported
   :widths: 50,10

    :c:func:`getegid`,no
    :c:func:`geteuid`,no
    :c:func:`getgid`,no
    :c:func:`getgroups`,no
    :c:func:`getlogin`,no
    :c:func:`getuid`,no
    :c:func:`setegid`,no
    :c:func:`seteuid`,no
    :c:func:`setgid`,no
    :c:func:`setuid`,no

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_USER_GROUPS`` Option
Group.

.. doxygengroup:: posix_option_group_user_groups
   :project: posix
