.. _posix_option_group_user_groups:

POSIX_USER_GROUPS
=================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_USER_GROUPS`.

Zephyr is a single-user system: the only identity is the privileged user (uid 0, gid 0), the
supplementary group set is empty, and the setters succeed only for that identity.

.. csv-table:: POSIX_USER_GROUPS
   :header: API, Supported
   :widths: 50,10

    :c:func:`getegid`,yes
    :c:func:`geteuid`,yes
    :c:func:`getgid`,yes
    :c:func:`getgroups`,yes
    :c:func:`getlogin`,yes
    :c:func:`getuid`,yes
    :c:func:`setegid`,yes
    :c:func:`seteuid`,yes
    :c:func:`setgid`,yes
    :c:func:`setuid`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_USER_GROUPS`` Option
Group.

.. doxygengroup:: posix_option_group_user_groups
   :project: posix
