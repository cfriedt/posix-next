.. _posix_option_chown_restricted:

_POSIX_CHOWN_RESTRICTED
=======================

The ``_POSIX_CHOWN_RESTRICTED`` Option does not add any interfaces; it indicates that use of
:c:func:`chown` is restricted to a process with appropriate privileges. It is unsupported
because Zephyr does not implement file ownership; see
:ref:`POSIX_FILE_ATTRIBUTES <posix_option_group_file_attributes>`.
