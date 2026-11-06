.. _posix_option_group_file_attributes:

POSIX_FILE_ATTRIBUTES
=====================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_FILE_ATTRIBUTES`.

Zephyr file systems store neither owners nor permission bits: every file belongs to the single
privileged user (uid 0, gid 0) and attribute changes on existing files succeed without effect
:ref:`†<posix_undefined_behaviour>`. See :ref:`posix_file_attributes_placeholder`.

.. csv-table:: POSIX_FILE_ATTRIBUTES
   :header: API, Supported
   :widths: 50,10

    :c:func:`chmod`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`chown`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`fchmod`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`fchown`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`umask`,yes :ref:`†<posix_undefined_behaviour>`

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_FILE_ATTRIBUTES`` Option
Group.
